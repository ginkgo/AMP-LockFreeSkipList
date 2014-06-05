/*
 * KLSMLocalityTaskStoragePlace.h
 *
 *  Created on: Sep 18, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGEPLACE_H_
#define KLSMLOCALITYTASKSTORAGEPLACE_H_

#include "KLSMLocalityTaskStorageItem.h"
#include "KLSMLocalityTaskStorageBlock.h"
#include "KLSMLocalityTaskStorageGlobalListItem.h"

#include <pheet/memory/BlockItemReuse/BlockItemReuseMemoryManager.h>
#include <pheet/memory/ItemReuse/ItemReuseMemoryManager.h>
#include <pheet/memory/Frame/FrameMemoryManagerFrame.h>
#include <pheet/memory/Frame/FrameMemoryManagerPlaceSingleton.h>

#include <limits>

namespace pheet {

template <class Pheet, class TaskStorage, class ParentTaskStoragePlace, class Strategy>
class KLSMLocalityTaskStoragePlace : public ParentTaskStoragePlace::BaseTaskStoragePlace {
public:
	typedef KLSMLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy> Self;

	typedef typename ParentTaskStoragePlace::BaseTaskStoragePlace BaseTaskStoragePlace;
	typedef FrameMemoryManagerFrame<Pheet> Frame;
	typedef FrameMemoryManagerPlaceSingleton<Pheet> FrameManager;
	typedef typename ParentTaskStoragePlace::BaseItem BaseItem;
	typedef KLSMLocalityTaskStorageItem<Pheet, Self, Frame, BaseItem, Strategy> Item;
	typedef KLSMLocalityTaskStorageBlock<Pheet, Item> Block;
	typedef KLSMLocalityTaskStorageGlobalListItem<Pheet, Block> GlobalListItem;

	typedef typename BaseItem::T T;

	typedef typename BlockItemReuseMemoryManager<Pheet, Item, KLSMLocalityTaskStorageItemReuseCheck<Item, Frame> >::template WithAmortization<2> ItemMemoryManager;
	typedef ItemReuseMemoryManager<Pheet, GlobalListItem, KLSMLocalityTaskStorageGlobalListItemReuseCheck<GlobalListItem> > GlobalListItemMemoryManager;

	typedef typename ParentTaskStoragePlace::PerformanceCounters PerformanceCounters;

	KLSMLocalityTaskStoragePlace(ParentTaskStoragePlace* parent_place)
	:pc(parent_place->pc),
	 parent_place(parent_place),
	 frame_man(Pheet::template place_singleton<FrameManager>()),
	 top_block_shared(nullptr), bottom_block_shared(nullptr),
	 best_block(nullptr), best_block_known(true),
	 needs_rescan(false),
	 current_frame(frame_man.next_frame()),
	 frame_used(0),
	 remaining_k(std::numeric_limits<size_t>::max()), tasks(0), sequential(Pheet::get_num_places() == 1) {

		// Get central task storage at end of initialization (not before,
		// since this place becomes visible as soon as we retrieve it)
		task_storage = TaskStorage::get(this, parent_place->get_central_task_storage(), created_task_storage);

		global_list = task_storage->get_global_list_head();

		// Fill in blocks usable for storing a single item
		bottom_block = find_free_block(0);
		top_block.store(bottom_block, std::memory_order_relaxed);
		bottom_block->mark_in_use();
	}

	~KLSMLocalityTaskStoragePlace() {
		for(auto i = blocks.begin(); i != blocks.end(); ++i) {
			delete *i;
		}

		if(created_task_storage) {
			delete task_storage;
		}

		pc.num_allocated_items.add(items.size());
	}

	GlobalListItem* create_global_list_item() {
		GlobalListItem* ret = &(global_list_items.acquire_item());
		ret->reset();
		return ret;
	}

	void push(Strategy&& strategy, T data) {
		Item& it = items.acquire_item();

		it.strategy = std::move(strategy);
		pheet_assert(!it.used_locally);
		it.used_locally = true;
		it.data = data;
		it.task_storage = task_storage;
		it.owner = this;
		if(frame_used >= 1024 || !current_frame->phase_change_required()) {
			// Exchange current frame every time there is congestion
			current_frame = frame_man.next_frame();
			frame_used = 0;
		}
		++frame_used;
		it.frame.store(current_frame, std::memory_order_release);
		it.phase = -1;
		current_frame->item_added();

		// Release, so that if item is seen as not taken by other threads
		// It is guaranteed to be newly initialized and the strategy set.
		it.taken.store(false, std::memory_order_release);

		put(&it, 0);

		parent_place->push(&it);

		// Bookkeeping for k-relaxation
		size_t k = it.strategy.get_k();
		bottom_block->added_local_item(k);
		remaining_k = std::min(remaining_k - 1, bottom_block->get_k());
		if(remaining_k == 0) {
			add_to_global_list();
		}
	}

	T pop(BaseItem* boundary) {
		Item* item = reinterpret_cast<Item*>(boundary);

		// Check whether boundary item is still available
		while(!item->is_taken()) {
			process_global_list();

			pheet_assert(best_block_known);
			// We can assume that boundary is stored in this task storage, so we just get the best item
			Block* best = best_block;

			if(best == nullptr || item->is_taken()) {
				return nullable_traits<T>::null_value;
			}

			Item* best_item = best->top();

			// Take does not do any deregistration, this is done by pop_taken_and_dead
			T data = best_item->take();

			// Check level of best block
			size_t level = best->get_level();

			// There is at least one taken task now, let's do a cleanup
			best->pop_taken_and_dead(this);
			best_block_known = false;
			best_block = nullptr;

			if(best->empty() || best->get_level() != level) {
				scan();
			}
			// Check whether we can merge blocks first
			else if(needs_rescan) {
				scan();
			}

			if(!best_block_known) {
				find_best_block();
			}

			// Will be automatically called in scan
	//		find_best_block();

			// Check whether we succeeded
			if(data != nullable_traits<T>::null_value) {
				tasks.store(tasks.load(std::memory_order_relaxed) - 1, std::memory_order_relaxed);
				return data;
			}
		}

		pheet_assert(best_block_known);
		// Linearized to check of boundary item
		return nullable_traits<T>::null_value;
	}

	T steal(BaseItem* boundary) {
		// TODO: make sure that items are not reread every time we steal. Could lead to bad scalability with large k if steal order coincides with FIFO order
		Item* item = reinterpret_cast<Item*>(boundary);

		// Should not be possible. Owner must have observed taken == false and will never go into steal
		// All owned items must be taken, otherwise no steal would be required
		// If it does, problems might occur with the reference counting
		pheet_assert(item->owner != this);

		// No need to do cheap taken check before, was already done by parent
		Frame* f = item->frame.load(std::memory_order_relaxed);
		size_t p;

		if(!frame_man.try_reg(f, p)) {
			// Spurious failure
			return nullable_traits<T>::null_value;
		}
		if(!item->is_taken() && f == item->frame.load(std::memory_order_relaxed)) {
			// Boundary item is still active. Put it in local queue, so we don't have to steal it another time
			// For the boundary item we ignore whether the item is dead, as long as it is not taken, it is
			// valid to be used
			put(item, p);

			// Parent place must know about the item as well, to omit multiple steals
			// (unless boundary has highest priority and does not create new tasks, then
			// stealing is inevitable for correct semantics)
			parent_place->push(item);

			// Only go through tasks of other thread if we have significantly less tasks stored locally
			// This is mainly to reduce spying in case the boundary items coincide with the
			// highest priority items, and k is large, where up to k items are looked at
			// every time an item is stolen.
			if(tasks.load(std::memory_order_relaxed) < (item->owner->tasks.load(std::memory_order_relaxed) << 1)) {
				// Now go through other tasks in the blocks of the given thread.
				// There is no guarantee that all tasks will be seen, since none of these tasks
				// would violate the k-requirements, this is not an issue
				// We call this spying
				Block* b = item->owner->top_block.load(std::memory_order_acquire);
				while(b != nullptr) {
					size_t filled = b->acquire_filled();
					for(size_t i = 0; i < filled; ++i) {
						Item* spy_item = b->get_item(i);
						// First do cheap taken check
						if(spy_item != item && !spy_item->is_taken()) {
							// Should never happen, we must have observed taken being set or we wouldn't
							// need to spy
							pheet_assert(spy_item->owner != this);
							Frame* spy_frame = spy_item->frame.load(std::memory_order_relaxed);
							size_t spy_p;

							// We do not need to see all items, so only proceed if we succeed
							// registering for the frame
							if(frame_man.try_reg(spy_frame, spy_p)) {

								// Frame might have changed between read and registration
								if(!spy_item->is_taken() && spy_item->frame.load(std::memory_order_relaxed) == spy_frame) {
									// Frame is still the same, we can proceed

									if(spy_item->is_dead()) {
										// We do not keep items that are already taken or marked as dead
										frame_man.rem_reg_buffered(spy_frame, spy_p);
									}
									else {
										pc.num_spied_tasks.incr();

										// Store item, but do not store in parent! (It's not a boundary after all!)
										put(spy_item, spy_p);
									}
								}
								else {
									// Frame has changed, just skip item
									frame_man.rem_reg_buffered(spy_frame, spy_p);

									// If item has changed, so has the block, so we can skip
									// the rest of the block
									break;
								}
							}
						}
					}

					// Not wait-free yet, since theoretically we might loop through blocks
					// of same size all the time (unrealistic, but not impossible)
					// Still, not really a problem, since it only depends on the behaviour of
					// the owner, and whp. we will not encounter more than a few cycles
					// before we reach a smaller block (we can never reach a larger block)
					b = b->acquire_next();
				}
			}

			// Now that we have spied some items from the owner of the boundary, we can just pop an item
			return pop(boundary);
		}
		else {
			// Frame has changed in the meantime, just deregister
			frame_man.rem_reg_buffered(f, p);
		}

		return nullable_traits<T>::null_value;
	}

	/*
	 * Override from parent
	 */
	void clean_up() {

	}

	PerformanceCounters pc;
private:
	void process_global_list() {
		GlobalListItem* next = global_list->move_next();
		while(next != nullptr) {
			global_list = next;

			Block* prev = nullptr;
			Block* b = global_list->acquire_block();
			// We might have to do this multiple times, since a block might be reused in the meantime
			// If block == nullptr all tasks in block have been taken or appear in a later global block
			// So we can safely skip it then
			while(b != nullptr && prev != b) {
				size_t filled = b->acquire_owned_filled();
				for(size_t i = 0; i < filled; ++i) {
					Item* spy_item = b->get_owned_item(i);
					pc.num_inspected_global_items.incr();

					// First do cheap taken check
					if(spy_item->owner != this && !spy_item->is_taken()) {
						Frame* spy_frame = spy_item->frame;
						size_t spy_p;

						frame_man.reg(spy_frame, spy_p);

						// Frame might have changed between read and registration
						if(!spy_item->is_taken() && spy_item->frame.load(std::memory_order_relaxed) == spy_frame) {
							// Frame is still the same, we can proceed

							if(spy_item->is_dead()) {
								// We do not keep items that are already taken or marked as dead
								frame_man.rem_reg_buffered(spy_frame, spy_p);
							}
							else {
								pc.num_spied_tasks.incr();
								pc.num_spied_global_tasks.incr();

								// Store item, but do not store in parent! (It's not a boundary after all!)
								put(spy_item, spy_p);
							}
						}
						else {
							// Frame has changed, just skip item
							frame_man.rem_reg_buffered(spy_frame, spy_p);

							// If item has changed, so has the block, so we can skip
							// the rest of the block. (either filled is now smaller or
							// block has been replaced)
							break;
						}
					}
				}


				prev = b;
				// Reload block, to see if it changed
				b = global_list->acquire_block();
			}

			next = global_list->move_next();
		}
	}

	void add_to_global_list() {
		process_global_list();

		Block* b = top_block.load(std::memory_order_relaxed);
		// Some blocks need to be made global (starting with oldest block)

		GlobalListItem* gli_first = nullptr;
		GlobalListItem* gli_last = nullptr;

		size_t next_id = global_list->get_id() + 1;

		// First construct a list of GlobalListItems locally
		while(b != nullptr) {
			// Check whether we are sequential is necessary since reference counting for
			// global list items only works correctly with P > 1
			if(!sequential && b->get_owned_filled() != 0) {
				GlobalListItem* gli = create_global_list_item();
				gli->update_block(b);
				gli->set_id(next_id);
				++next_id;

				pc.num_global_blocks.incr();
				if(gli_last == nullptr) {
					gli_first = gli;
					gli_last = gli;
				}
				else {
					gli_last->local_link(gli);
					gli_last = gli;
				}
				b->mark_newly_global(gli);
			}
			else {
				b->mark_newly_global(nullptr);
			}
			b = b->get_next();
		}

		// And then connect it to the global list
		if(gli_first != nullptr) {
			while(gli_first != nullptr && !global_list->link(gli_first)) {
				// Global List has been extended by other thread, process it first
				process_global_list();

				// Go through list again to update ids and clean out potentially unnecessary items
				GlobalListItem* prev = nullptr;
				GlobalListItem* curr = gli_first;
				next_id = global_list->get_id() + 1;
				while(curr != nullptr) {
					GlobalListItem* next = curr->get_next();
					// Skip blocks that are empty now
					if(curr->get_block() == nullptr) {
						if(prev == nullptr) {
							gli_first = next;
						}
						else {
							prev->local_link(next);
						}
						curr->mark_reusable();
						curr = next;
					}
					else {
						curr->set_id(next_id);
						++next_id;
						prev = curr;
						curr = next;
					}
				}
				gli_last = prev;
			}
			if(gli_last != nullptr) {
				global_list = gli_last;
			}
		}

		// Now add each block to shared list
		b = top_block.load(std::memory_order_relaxed);
		while(b != nullptr) {
			Block* next = b->get_next();
			pheet_assert(next == nullptr || !next->reusable());
			pheet_assert(next != nullptr || b == bottom_block);

			if(!b->empty()) {
				link_to_shared_list(b);
			} else {
				top_block.store(next, std::memory_order_relaxed);
				b->reset();
			}
			b = next;
		}

		// Now find new block for local list and reset local list
		Block* new_bb = find_free_block((blocks.size() >> 2) - 1);
		new_bb->mark_in_use();
		pheet_assert(new_bb->get_prev() == nullptr);
		pheet_assert(new_bb->get_next() == nullptr);

		bottom_block = new_bb;
		top_block.store(bottom_block, std::memory_order_release);

		// Restart counting
		remaining_k = std::numeric_limits<size_t>::max();

		if(!best_block_known) {
			find_best_block();
		}
	}

	void link_to_shared_list(Block* block) {
		Block* next = nullptr;
		Block* b = bottom_block_shared;

		while(b != nullptr && block->get_level() > b->get_max_level() &&
				(block->get_level() > b->get_level() || b->empty())) {
			next = b;
			b = b->get_prev();
		}
		while(b != nullptr && b->empty()) {
			Block* p = b->get_prev();
			b->reset();
			// No need to update pointers, will be corrected when linking in block anyway
			b = p;
		}
		if(b != nullptr && block->get_level() < b->get_level() &&
				block->get_max_level() >= b->get_max_level()) {
			// We need to guarantee that each block size only appears in shared list once
			// and that they are ordered by size
			// In case no merge will occur we need to copy the data into a smaller block to
			// guarantee this
			Block* new_b = find_free_block(block->get_level());
			new_b->copy_items(block);
			new_b->mark_in_use();
			GlobalListItem* gli = block->get_global_list_item();
			if(gli != nullptr) {
				gli->update_block(new_b);
			}
			if(block == best_block) {
				best_block = new_b;
			}
			block->reset();
			block = new_b;
		}

		if(next == nullptr) {
			bottom_block_shared = block;
		}
		else {
			next->set_prev(block);
		}
		block->set_prev(b);
		block->release_next(next);
		if(b == nullptr) {
			top_block_shared = block;
		}
		else {
			b->set_next(block);

			if(b->get_level() <= block->get_level() || b->get_max_level() <= block->get_max_level()) {
				block = merge_shared(block);

				if(block->empty()) {
					Block* p = block->get_prev();
					Block* n = block->get_next();
					if(n != nullptr) {
						n->set_prev(p);
					}
					else {
						pheet_assert(bottom_block_shared == block);
						bottom_block_shared = p;
					}
					if(p != nullptr) {
						p->set_next(n);
					}
					else {
						pheet_assert(top_block_shared == block);
						top_block_shared = n;
					}
					block->reset();
					pheet_assert(bottom_block_shared != nullptr || top_block_shared == nullptr);
				}
			}
		}
		pheet_assert(block->get_prev() == nullptr || block->get_prev()->get_max_level() > block->get_max_level());
		pheet_assert(block->get_next() == nullptr || block->get_next()->get_max_level() < block->get_max_level());

		pheet_assert(bottom_block_shared == nullptr || bottom_block_shared->get_next() == nullptr);
		pheet_assert(top_block_shared == nullptr || top_block_shared->get_prev() == nullptr);
	}

	void put(Item* item, size_t p) {
		pheet_assert(best_block_known);

		tasks.store(tasks.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);

		Block* bb = bottom_block;
		pheet_assert(bb->get_next() == nullptr);
		pheet_assert(!bb->reusable());
		if(!bb->try_put(item, item->owner == this, p)) {
			// Lazy merging, blocks will only be merged when looking for the best block
			// or when running out of blocks to use, or when the guarantees about blocks used would be violated

			if(bb->get_prev() != nullptr && bb->get_prev()->get_level() <= bb->get_max_level()) {
				scan();
				bb = bottom_block;
			}

			size_t l = bb->get_level();
			pheet_assert(bb->get_prev() == nullptr || bb->get_prev()->get_max_level() > l);
			size_t offset = l << 2;

			Block* new_bb = nullptr;
			for(int i = 0; i < 4; ++i) {
				if(blocks[offset + i]->reusable()) {
					new_bb = blocks[offset + i];
					break;
				}
			}
			new_bb->mark_in_use();
			new_bb->set_prev(bb);
			pheet_assert(new_bb->get_next() == nullptr);
			// Make sure that if block is seen, so are the changes to the block
			pheet_assert(bb != bottom_block_shared);
			bb->release_next(new_bb);
			bottom_block = new_bb;

			// We know we have space in here, so no try_put needed
			bottom_block->put(item, item->owner == this, p);

			if(best_block == nullptr || (best_block != bottom_block && item->strategy.prioritize(best_block->top()->strategy))) {
				best_block = bottom_block;
			}
		}
		else if(best_block == nullptr || (best_block != bb && item->strategy.prioritize(best_block->top()->strategy))) {
			best_block = bb;
		}
	}

	Block* merge(Block* block) {
		// Only do this if merge is actually required
		pheet_assert(block->get_prev() != nullptr && block->get_level() >= block->get_prev()->get_level());

		// Block should not be empty or try_put would have succeeded
		pheet_assert(!block->empty());

		// Never go below this level to ensure we never destroy the guarantees that max_levels are strictly
		// increasing and that never more than 2 blocks in local list have the same max_level
		size_t l_min = block->get_max_level();

		Block* pre_merge = block->get_next();
		Block* last_merge = block->get_prev();
		pheet_assert(!last_merge->empty());
		pheet_assert(block == last_merge->get_next());
		Block* merged = find_free_block(std::max(l_min, block->get_level() + 1));
		merged->merge_into(last_merge, block, this);
		merged->mark_in_use();
		pheet_assert(merged->get_filled() <= last_merge->get_filled() + block->get_filled());
		pheet_assert(merged->get_owned_filled() <= last_merge->get_owned_filled() + block->get_owned_filled());
		pheet_assert(merged->get_k() >= std::min(last_merge->get_k() - block->get_owned_filled(), block->get_k()));

		Block* prev = last_merge->get_prev();
		pheet_assert(prev == nullptr || prev->get_next() == last_merge);

		// Blocks might already have global list items when they are being prepared for publishing
		GlobalListItem* gli = block->get_global_list_item();
		if(gli == nullptr) {
			gli = last_merge->get_global_list_item();
		}

		while(prev != nullptr && !merged->empty() &&
				merged->get_level() >= prev->get_level()) {
			last_merge = prev;

			pheet_assert(!last_merge->empty());
			pheet_assert(merged->get_max_level() <= prev->get_max_level());

			// Only merge if the other block is not empty
			// Each subsequent merge takes a block one greater than the previous one to ensure we don't run out of blocks
			size_t l = std::max(l_min, std::max(last_merge->get_level() + 1, merged->get_level() + 1));
			Block* merged2 = find_free_block(l);
			if(merged2 == nullptr) {
				// Can sometimes happen if previous merge used exactly the same block size and both predecessors have
				// the same block size
				pheet_assert(l < merged->get_max_level() + 1);
				// We can assume that there will be a block available since a block may only occur
				// at most once in the shared list, and twice in the local list
				merged2 = find_free_block(l + 1);
			}
			pheet_assert(merged2 != bottom_block_shared);
			merged2->merge_into(last_merge, merged, this);

			if(gli == nullptr) {
				gli = last_merge->get_global_list_item();
			}
			pheet_assert(merged2->get_filled() <= last_merge->get_filled() + merged->get_filled());
			pheet_assert(merged2->get_owned_filled() <= last_merge->get_owned_filled() + merged->get_owned_filled());
			pheet_assert(merged2->get_k() >= std::min(last_merge->get_k() - merged->get_owned_filled(), merged->get_k()));

			// The merged block was never made visible, we can reset it at any time
			pheet_assert(merged != bottom_block);
			pheet_assert(merged != bottom_block_shared);
			pc.num_merges.add(merged->get_filled());
			merged->reset();
			merged = merged2;
			merged->mark_in_use();

			prev = last_merge->get_prev();
			pheet_assert(prev == nullptr || prev->get_next() == last_merge);
			pheet_assert(prev == nullptr || prev->get_prev() == nullptr ||
					prev->get_prev()->get_max_level() > merged->get_max_level() || prev->get_level() <= merged->get_level());
		}
		pheet_assert(prev == nullptr || merged->get_max_level() <= prev->get_max_level());

		// Will be released when merged block is made visible through update of top block or next pointer
		merged->set_next(block->get_next());

		// Now we need to make new blocks visible, and reset the old blocks
		if(prev != nullptr) {
			merged->set_prev(prev);

			// Make all changes visible now using a release
			pheet_assert(prev != bottom_block_shared);
			prev->release_next(merged);

			pheet_assert(prev->get_prev() == nullptr || prev->get_prev()->get_max_level() > merged->get_max_level());
		}
		else {
			// Everything merged into one block
			// Make all changes visible now using a release
			top_block.store(merged, std::memory_order_release);
		}

		if(pre_merge == nullptr) {
			bottom_block = merged;
		}
		else {
			pheet_assert(pre_merge != top_block.load(std::memory_order_relaxed));
			pheet_assert(pre_merge != top_block_shared);
			pre_merge->set_prev(merged);
			pheet_assert(pre_merge->get_max_level() < merged->get_max_level() ||
					merged->get_prev() == nullptr ||
					merged->get_prev()->get_max_level() > merged->get_max_level());
		}

		// Now we need to update the global list item if necessary
		if(gli != nullptr) {
			gli->update_block(merged);
		}

		// Now reset old blocks
		while(last_merge != pre_merge) {
			Block* next = last_merge->get_next();
			if(last_merge == best_block) {
				best_block = nullptr;
				best_block_known = false;
			}
			pc.num_merges.add(last_merge->get_filled());
			pheet_assert(last_merge != bottom_block);
			pheet_assert(last_merge != bottom_block_shared);
			last_merge->reset();
			last_merge = next;
		}

		return merged;
	}

	Block* merge_shared(Block* block) {
		// Only do this if merge is actually required
		pheet_assert(block->get_prev() != nullptr &&
				(block->get_level() >= block->get_prev()->get_level() ||
						block->get_max_level() >= block->get_prev()->get_max_level()));

		pheet_assert(bottom_block_shared->get_next() == nullptr);
		pheet_assert(top_block_shared->get_prev() == nullptr);

		// Block should not be empty or try_put would have succeeded
		pheet_assert(!block->empty());

		Block* pre_merge = block->get_next();
		Block* last_merge = block->get_prev();
		pheet_assert(!last_merge->empty());
		pheet_assert(block == last_merge->get_next());

		// Never go below this level to ensure we never destroy the guarantees that max_levels are strictly
		// increasing and that never more than 2 blocks in local list have the same max_level
		size_t l_min = std::min(block->get_max_level(), last_merge->get_max_level());

		// Choose a block big enough to fit data of both blocks, but never smaller than the smaller of
		// the two blocks (No point in using a smaller block, larger is definitely free)
		Block* merged = find_free_block(std::max(std::max(block->get_level() + 1, last_merge->get_level() + 1), l_min));
		merged->merge_into(last_merge, block, this);
		merged->mark_in_use();
		pheet_assert(merged->get_filled() <= last_merge->get_filled() + block->get_filled());
		pheet_assert(merged->get_owned_filled() <= last_merge->get_owned_filled() + block->get_owned_filled());
		pheet_assert(merged->get_k() >= std::min(last_merge->get_k() - block->get_owned_filled(), block->get_k()));

		Block* prev = last_merge->get_prev();
		pheet_assert(prev == nullptr || prev->get_next() == last_merge);

		GlobalListItem* gli = block->get_global_list_item();
		if(gli == nullptr) {
			gli = last_merge->get_global_list_item();
		}

		while(prev != nullptr && !merged->empty() &&
				(merged->get_level() >= prev->get_level() || merged->get_max_level() >= prev->get_max_level())) {
			last_merge = prev;

			if(!last_merge->empty()) {
				// Only merge if the other block is not empty
				size_t l = std::max(l_min, std::max(last_merge->get_level() + 1, merged->get_level() + 1));
				Block* merged2 = find_free_block(l);
				if(merged2 == nullptr) {
					// May happen if one block of same size as merged block is in list, and 2 blocks of same
					// size are used in local list
					pheet_assert(l == merged->get_max_level());
					// A block with l + 1 is guaranteed to be available (the 4th block is only for merging,
					// so, conflict only occurs when a merged block is merged again)
					merged2 = find_free_block(l + 1);
				}
				merged2->merge_into(last_merge, merged, this);

				if(gli == nullptr) {
					gli = last_merge->get_global_list_item();
				}
				pheet_assert(merged2->get_filled() <= last_merge->get_filled() + merged->get_filled());
				pheet_assert(merged2->get_owned_filled() <= last_merge->get_owned_filled() + merged->get_owned_filled());
				pheet_assert(merged2->get_k() >= std::min(last_merge->get_k() - merged->get_owned_filled(), merged->get_k()));

				// The merged block was never made visible, we can reset it at any time
				pheet_assert(merged != bottom_block);
				pheet_assert(merged != bottom_block_shared);
				pc.num_merges.add(merged->get_filled());
				merged->reset();
				merged = merged2;
				merged->mark_in_use();
			}

			prev = last_merge->get_prev();
			pheet_assert(prev == nullptr || prev->get_next() == last_merge);
		}
		pheet_assert(prev == nullptr || merged->get_max_level() <= prev->get_max_level());

		// Will be released when merged block is made visible through update of top block or next pointer
		merged->set_next(block->get_next());

		// Now we need to make new blocks visible, and reset the old blocks
		if(prev != nullptr) {
			merged->set_prev(prev);

			// Make all changes visible now using a release
			prev->release_next(merged);
		}
		else {
			// Everything merged into one block
			// Make all changes visible now using a release
			top_block_shared = merged;
		}

		if(pre_merge == nullptr) {
			bottom_block_shared = merged;
		}
		else {
			pre_merge->set_prev(merged);
		}

		// Now we need to update the global list item if necessary
		if(gli != nullptr) {
			gli->update_block(merged);
		}

		// Now reset old blocks
		while(last_merge != pre_merge) {
			Block* next = last_merge->get_next();
			if(last_merge == best_block) {
				best_block = nullptr;
				best_block_known = false;
			}
			pc.num_merges.add(last_merge->get_filled());
			pheet_assert(last_merge != bottom_block_shared);
			last_merge->reset();
			last_merge = next;
		}
		pheet_assert(bottom_block_shared->get_next() == nullptr);
		pheet_assert(top_block_shared->get_prev() == nullptr);

		return merged;
	}

	void find_best_block() {
		Block* b = bottom_block;
		pheet_assert(b != nullptr);

		do {
			if(!b->empty() &&
					(best_block == nullptr ||
						b->top()->strategy.prioritize(best_block->top()->strategy))) {
				best_block = b;
			}
			b = b->get_prev();
		} while(b != nullptr);

		b = bottom_block_shared;
		while(b != nullptr) {
			if(!b->empty() &&
					(best_block == nullptr ||
						b->top()->strategy.prioritize(best_block->top()->strategy))) {
				best_block = b;
			}
			b = b->get_prev();
		}

		if(best_block == nullptr) {
			remaining_k = std::numeric_limits<size_t>::max();
		}

		best_block_known = true;
	}

	/*
	 * Goes through local lists of blocks and merges all mergeable blocks and removes empty
	 * blocks
	 */
	void scan() {
		// Recount number of tasks (might change due to merging dead tasks, so it's easiest to recalculate)
		size_t num_tasks = 0;
		remaining_k = std::numeric_limits<size_t>::max();

		Block* prev = nullptr;
		Block* b = top_block.load(std::memory_order_relaxed);
		pheet_assert(b != nullptr);

		do {
			num_tasks += b->get_filled();

			Block* next = b->get_next();
			if(b->empty()) {
				if(next != nullptr) {
					next->set_prev(prev);
					if(prev != nullptr) {
						// No need to order, we didn't change the block
						prev->set_next(next);
					}
					else {
						// No need to order, we didn't change the block
						top_block.store(next, std::memory_order_relaxed);
					}
					b->reset();
					b = next;
				}
				else {
					// Never remove first (bottom) block if it is empty, it is used for adding items
					pheet_assert(next == nullptr || next->get_prev() == b);
					prev = b;
					b = next;
				}
			}
			else if(prev != nullptr && b->get_level() >= prev->get_level()) {
				pheet_assert(remaining_k >= b->get_owned_filled());
				remaining_k = std::min(remaining_k - b->get_owned_filled(), b->get_k());
				b = merge(b);

				if(b->empty() && next != nullptr) {
					prev = b->get_prev();
					next->set_prev(prev);
					if(prev != nullptr) {
						// No need to order, we didn't change the block
						prev->set_next(next);
					}
					else {
						// No need to order, we didn't change the block
						top_block.store(next, std::memory_order_relaxed);
					}
					b->reset();
					b = next;
				}
				else {
					prev = b;
					b = next;
				}
			}
			else {
				pheet_assert(remaining_k >= b->get_owned_filled());
				remaining_k = std::min(remaining_k - b->get_owned_filled(), b->get_k());

				pheet_assert(next == nullptr || next->get_prev() == b);
				prev = b;
				b = next;
			}
		} while(b != nullptr);
		pheet_assert(prev == bottom_block);

		prev = nullptr;
		b = top_block_shared;

		// Now go through list of shared blocks
		// No need to order memory accesses list is not accessed globally through these pointers
		while(b != nullptr) {
			num_tasks += b->get_filled();

			Block* next = b->get_next();
			if(b->empty()) {
				if(next != nullptr) {
					pheet_assert(!next->reusable());
					next->set_prev(prev);
					if(prev != nullptr) {
						pheet_assert(!prev->reusable());
						prev->set_next(next);
					}
					else {
						top_block_shared = next;
					}
				}
				else if(prev != nullptr) {
					pheet_assert(!prev->reusable());
					prev->set_next(nullptr);
					pheet_assert(bottom_block_shared == b);
					bottom_block_shared = prev;
				}
				else {
					pheet_assert(top_block_shared == b);
					pheet_assert(bottom_block_shared == b);
					top_block_shared = nullptr;
					bottom_block_shared = nullptr;
				}
				b->reset();
				b = next;
			}
			else if(prev != nullptr && (b->get_level() >= prev->get_level() || b->get_max_level() >= prev->get_max_level())) {
				b = merge_shared(b);

				// Repeat cycle since block might now be empty
				pheet_assert(b->get_next() == next);
				prev = b->get_prev();
			}
			else {
				pheet_assert(next == nullptr || next->get_prev() == b);
				prev = b;
				b = next;
			}
		}

		tasks.store(num_tasks, std::memory_order_relaxed);
		needs_rescan = false;

		if(!best_block_known) {
			find_best_block();
		}
	}

	Block* find_free_block(size_t level) {
		size_t offset = level << 2;

		if(offset >= blocks.size()) {
			do {
				size_t l = blocks.size() >> 2;
				size_t s = (1 << l);
				for(int i = 0; i < 4; ++i) {
					blocks.push_back(new Block(s, l));
					pc.num_blocks_created.incr();
				}
			}while(offset >= blocks.size());

			return blocks.back();
		}
		for(int i = 0; i < 4; ++i) {
			if(blocks[offset + i]->reusable()) {
				return blocks[offset + i];
			}
		}
		return nullptr;
	}

	ParentTaskStoragePlace* parent_place;
	TaskStorage* task_storage;
	bool created_task_storage;

	ItemMemoryManager items;
	FrameManager& frame_man;
	GlobalListItemMemoryManager global_list_items;

	// Stores 3 blocks per level.
	// 2 would be enough in general, but with 3 less synchronization is required
	std::vector<Block*> blocks;
	std::atomic<Block*> top_block;
	Block* bottom_block;
	Block* top_block_shared;
	Block* bottom_block_shared;
	Block* best_block;
	bool best_block_known;
	bool needs_rescan;

	Frame* current_frame;
	size_t frame_used;

//	Block* best_block;

	size_t remaining_k;
	std::atomic<size_t> tasks;

	GlobalListItem* global_list;
	bool sequential;
};

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGEPLACE_H_ */
