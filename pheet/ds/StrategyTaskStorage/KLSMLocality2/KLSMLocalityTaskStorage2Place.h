/*
 * KLSMLocalityTaskStorage2Place.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGE2PLACE_H_
#define KLSMLOCALITYTASKSTORAGE2PLACE_H_

#include "KLSMLocalityTaskStorage2Item.h"
#include "KLSMLocalityTaskStorage2Block.h"
#include "KLSMLocalityTaskStorage2BlockAnnouncement.h"

#include <pheet/memory/BlockItemReuse/BlockItemReuseMemoryManager.h>
#include <pheet/memory/ItemReuse/ItemReuseMemoryManager.h>

#include <limits>
#include <atomic>

namespace pheet {

template <class Pheet, class TaskStorage, class ParentTaskStoragePlace, class Strategy>
class KLSMLocalityTaskStorage2Place : public ParentTaskStoragePlace::BaseTaskStoragePlace {
public:
	typedef KLSMLocalityTaskStorage2Place<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy> Self;

	typedef typename ParentTaskStoragePlace::BaseTaskStoragePlace BaseTaskStoragePlace;
	typedef typename ParentTaskStoragePlace::BaseItem BaseItem;
	typedef KLSMLocalityTaskStorage2Item<Pheet, Self, BaseItem, Strategy> Item;
	typedef KLSMLocalityTaskStorage2Block<Pheet, Item> Block;
	typedef KLSMLocalityTaskStorage2BlockAnnouncement<Pheet, Block> BlockAnnouncement;

	typedef typename BaseItem::T T;

	typedef typename BlockItemReuseMemoryManager<Pheet, Item, KLSMLocalityTaskStorage2ItemReuseCheck<Item> > ItemMemoryManager;
	typedef typename ItemReuseMemoryManager<Pheet, BlockAnnouncement, KLSMLocalityTaskStorage2BlockAnnouncementReuseCheck<BlockAnnouncement> > BlockAnnouncementMemoryManager;

	typedef typename ParentTaskStoragePlace::PerformanceCounters PerformanceCounters;

	KLSMLocalityTaskStorage2Place(ParentTaskStoragePlace* parent_place)
	:pc(parent_place->pc),
	 parent_place(parent_place),
	 frame_used(0),
	 tasks(0),
	 unannounced_tasks(0),
	 block_announcement_head(nullptr),
	 block_announcement_tail(nullptr) {

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

		BlockAnnouncement* ba = &(block_announcements.acquite_item());
		block_announcement_head = ba;
		block_announcement_tail.store(ba, std::memory_order_relaxed);
	}
	~KLSMLocalityTaskStorage2Place() {
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

		// Release, so that if item is seen as not taken by other threads
		// It is guaranteed to be newly initialized and the strategy set.
		it.taken.store(false, std::memory_order_release);

		put(&it);

		parent_place->push(&it);
	}

	T pop(BaseItem* boundary) {
		Item* item = reinterpret_cast<Item*>(boundary);

		pheet_assert(boundary != nullptr);
		pheet_assert(item->owner == this);

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
		Item* item = reinterpret_cast<Item*>(boundary);

		pheet_assert(boundary != nullptr);
		pheet_assert(item->owner != this);
		// Might not be empty yet, since empty blocks might not have been cleaned up
	//	pheet_assert(size() == 0);

		T data = item->take();
		return data;
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
	void put(Item* item) {
		tasks.store(tasks.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);

		Block* bb = bottom_block;
		pheet_assert(bb->get_next() == nullptr);
		pheet_assert(!bb->reusable());
		if(!bb->try_put(item)) {
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
			new_bb->put(item);

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

	Self* get_ith_partner(size_t i) {
		procs_t id = Pheet::get_place()->get_ith_nearest_partner(i);
		return task_storage->get_place(i);
	}

	ParentTaskStoragePlace* parent_place;
	TaskStorage* task_storage;
	bool created_task_storage;

	ItemMemoryManager items;
	BlockAnnouncementMemoryManager block_announcements;

	// Stores 3 blocks per level.
	// 2 would be enough in general, but with 3 less synchronization is required
	std::vector<Block*> blocks;
	std::atomic<Block*> top_block;
	Block* bottom_block;

//	Block* best_block;

	// Is updated on pop operations
	std::atomic<size_t> tasks;
	size_t unannounced_tasks;

	BlockAnnouncement* block_announcement_head;
	std::atomic<BlockAnnouncement*> block_announcement_tail;
};

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGE2PLACE_H_ */
