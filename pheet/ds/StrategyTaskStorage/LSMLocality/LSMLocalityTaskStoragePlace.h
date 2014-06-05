/*
 * LSMLocalityTaskStoragePlace.h
 *
 *  Created on: Sep 18, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LSMLOCALITYTASKSTORAGEPLACE_H_
#define LSMLOCALITYTASKSTORAGEPLACE_H_

#include "LSMLocalityTaskStorageItem.h"
#include "LSMLocalityTaskStorageBlock.h"

#include <pheet/memory/BlockItemReuse/BlockItemReuseMemoryManager.h>
#include <pheet/memory/Frame/FrameMemoryManagerFrame.h>
#include <pheet/memory/Frame/FrameMemoryManagerPlaceSingleton.h>

#include <limits>

namespace pheet {

template <class Pheet, class TaskStorage, class ParentTaskStoragePlace, class Strategy>
class LSMLocalityTaskStoragePlace : public ParentTaskStoragePlace::BaseTaskStoragePlace {
public:
	typedef LSMLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy> Self;

	typedef typename ParentTaskStoragePlace::BaseTaskStoragePlace BaseTaskStoragePlace;
	typedef FrameMemoryManagerFrame<Pheet> Frame;
	typedef FrameMemoryManagerPlaceSingleton<Pheet> FrameManager;
	typedef typename ParentTaskStoragePlace::BaseItem BaseItem;
	typedef LSMLocalityTaskStorageItem<Pheet, Self, Frame, BaseItem, Strategy> Item;
	typedef LSMLocalityTaskStorageBlock<Pheet, Item> Block;

	typedef typename BaseItem::T T;

	typedef typename BlockItemReuseMemoryManager<Pheet, Item, LSMLocalityTaskStorageItemReuseCheck<Item, Frame> >::template WithAmortization<2> ItemMemoryManager;

	typedef typename ParentTaskStoragePlace::PerformanceCounters PerformanceCounters;

	LSMLocalityTaskStoragePlace(ParentTaskStoragePlace* parent_place)
	:pc(parent_place->pc),
	 parent_place(parent_place),
	 frame_man(Pheet::template place_singleton<FrameManager>()),
	 current_frame(frame_man.next_frame()),
	 frame_used(0),
	 tasks(0) {

		// Get central task storage at end of initialization (not before,
		// since this place becomes visible as soon as we retrieve it)
		task_storage = TaskStorage::get(this, parent_place->get_central_task_storage(), created_task_storage);

		// Fill in blocks usable for storing a single item
		// Might make sense to precreate more, but for now let's leave it at that
		blocks.push_back(new Block(1));
		blocks.push_back(new Block(1));
		blocks.push_back(new Block(1));

		top_block.store(blocks[0], std::memory_order_relaxed);
		bottom_block = blocks[0];
		bottom_block->mark_in_use();
	}
	~LSMLocalityTaskStoragePlace() {
		for(auto i = blocks.begin(); i != blocks.end(); ++i) {
			delete *i;
		}

		if(created_task_storage) {
			delete task_storage;
		}

		pc.num_allocated_items.add(items.size());
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
	}

	T pop(BaseItem* boundary) {
		Item* item = reinterpret_cast<Item*>(boundary);

		// Check whether boundary item is still available
		while(!item->is_taken()) {

			// We can assume that boundary is stored in this task storage, so we just get the best item
			Block* best = find_best_block();

			if(best == nullptr || item->is_taken()) {
				return nullable_traits<T>::null_value;
			}

			Item* best_item = best->top();

			// Take does not do any deregistration, this is done by pop_taken_and_dead
			T data = best_item->take();

			// There is at least one taken task now, let's do a cleanup
			best->pop_taken_and_dead(this);

			// Check whether we succeeded
			if(data != nullable_traits<T>::null_value) {
				tasks.store(tasks.load(std::memory_order_relaxed) - 1, std::memory_order_relaxed);
				return data;
			}
		}

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

	size_t size() {
		return tasks.load(std::memory_order_relaxed);
	}

	PerformanceCounters pc;
private:
	void put(Item* item, size_t p) {
		tasks.store(tasks.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);

		Block* bb = bottom_block;
		pheet_assert(bb->get_next() == nullptr);
		pheet_assert(!bb->reusable());
		if(!bb->try_put(item, p)) {
			// Check whether a merge is required
			if(bb->get_prev() != nullptr && bb->get_level() >= bb->get_prev()->get_level()) {
				bb = merge(bb);
				bottom_block = bb;
			}

			size_t l = bb->get_level();
			size_t offset = std::min((l*3) + 2, blocks.size() - 1);

			Block* new_bb = blocks[offset];
			while(!new_bb->reusable()) {
				--offset;
				new_bb = blocks[offset];
			}
			new_bb->mark_in_use();
			new_bb->set_prev(bb);
			pheet_assert(new_bb->get_next() == nullptr);
			// Make sure that if block is seen, so are the changes to the block
			bb->release_next(new_bb);

			// We know we have space in here, so no try_put needed
			new_bb->put(item, p);

			bottom_block = new_bb;
		}
	}

	Block* merge(Block* block) {
		// Only do this if merge is actually required
		pheet_assert(block->get_prev() != nullptr && block->get_level() >= block->get_prev()->get_level());

		// Block should not be empty or try_put would have succeeded
		pheet_assert(!block->empty());

		Block* pre_merge = block->get_next();
		Block* last_merge = block->get_prev();
		pheet_assert(block == last_merge->get_next());
		Block* merged = find_free_block(block->get_level() + 1);
		merged->merge_into(last_merge, block, this);
		merged->mark_in_use();

		Block* prev = last_merge->get_prev();
		pheet_assert(prev == nullptr || prev->get_next() == last_merge);

		while(prev != nullptr && merged->get_level() >= prev->get_level()) {
			last_merge = prev;

			if(!last_merge->empty()) {
				// Only merge if the other block is not empty
				Block* merged2 = find_free_block(merged->get_level() + 1);
				merged2->merge_into(last_merge, merged, this);

				// The merged block was never made visible, we can reset it at any time
				pc.num_merges.add(merged->get_filled());
				merged->reset();
				merged = merged2;
				merged->mark_in_use();
			}
			prev = last_merge->get_prev();
			pheet_assert(prev == nullptr || prev->get_next() == last_merge);
		}

		// Will be released when merged block is made visible through update of top block or next pointer
		merged->set_next(block->get_next());

		// Now we need to make new blocks visible, and reset the old blocks
		if(prev == nullptr) {
			// Everything merged into one block
			// Make all changes visible now using a release
			top_block.store(merged, std::memory_order_release);
		}
		else {
			merged->set_prev(prev);

			// Make all changes visible now using a release
			prev->release_next(merged);
		}
		// Now reset old blocks
		while(last_merge != pre_merge) {
			Block* next = last_merge->get_next();
			pc.num_merges.add(last_merge->get_filled());
			last_merge->reset();
			last_merge = next;
		}

		return merged;
	}

	Block* find_best_block() {
		// Recount number of tasks (might change due to merging, dead tasks, so it's easiest to recalculate)
		size_t num_tasks = 0;

		Block* b = bottom_block;
		if(b->empty()) {
			Block* prev = b->get_prev();
			while(prev != nullptr && prev->empty()) {
				bottom_block = prev;
				prev->set_next(nullptr);
				b->reset();
				prev = prev->get_prev();
			}
			b = prev;
		}

		if(b == nullptr) {
		//	bottom_block = top_block.load(std::memory_order_relaxed);
			return nullptr;
		}

		pheet_assert(!b->empty());
		Block* best = b;
		Block* next = b;
		num_tasks += b->get_filled();
		b = b->get_prev();

		while(b != nullptr) {
			pheet_assert(!b->reusable());

			Block* p = b->get_prev();
			pheet_assert(p == nullptr || p->get_next() == b);
			if(b->empty()) {
				next->set_prev(p);
				if(p == nullptr) {
					// No need to order, we didn't change the block
					top_block.store(next, std::memory_order_relaxed);

					// Contains a release
					b->reset();

					tasks = num_tasks;

					// End of list, just return best
					return best;
				}
				else {
					// No need to order, we didn't change the block
					p->set_next(next);

					// Contains a release
					b->reset();

					b = p;
				}
			}
			else if(p != nullptr && b->get_level() >= p->get_level()) {
				pheet_assert(b != best);
				b = merge(b);
				pheet_assert(!b->reusable());
				pheet_assert(!best->reusable());
				next->set_prev(b);

				// We need to redo cycle, since merged block might be empty!
			}
			else {
				if(b->top()->strategy.prioritize(best->top()->strategy)) {
					best = b;
				}
				num_tasks += b->get_filled();
				next = b;
				b = p;
			}
		}

		tasks.store(num_tasks, std::memory_order_relaxed);

		return best;
	}

	Block* find_free_block(size_t level) {
		size_t offset = level * 3;

		if(offset >= blocks.size()) {
			do {
				size_t l = blocks.size() / 3;
				blocks.push_back(new Block(1 << l));
			}while(offset >= blocks.size());

			return blocks.back();
		}
		while(!blocks[offset]->reusable()) {
			++offset;
			if(blocks.size() == offset) {
				size_t l = offset / 3;
				Block* ret = new Block(1 << l);
				blocks.push_back(ret);
				return ret;
			}
		}
		return blocks[offset];
	}

	ParentTaskStoragePlace* parent_place;
	TaskStorage* task_storage;
	bool created_task_storage;

	ItemMemoryManager items;
	FrameManager& frame_man;

	// Stores 3 blocks per level.
	// 2 would be enough in general, but with 3 less synchronization is required
	std::vector<Block*> blocks;
	std::atomic<Block*> top_block;
	Block* bottom_block;

	Frame* current_frame;
	size_t frame_used;

//	Block* best_block;

	std::atomic<size_t> tasks;
};

} /* namespace pheet */
#endif /* LSMLOCALITYTASKSTORAGEPLACE_H_ */
