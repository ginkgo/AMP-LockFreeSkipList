/*
 * LSMLocalityTaskStorageBlock.h
 *
 *  Created on: Sep 24, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LSMLOCALITYTASKSTORAGEBLOCK_H_
#define LSMLOCALITYTASKSTORAGEBLOCK_H_

#include <atomic>
#include <algorithm>

#include <pheet/memory/Frame/FrameMemoryManagerPlaceSingleton.h>

namespace pheet {

/*
 * Block for task storage. Stores items in sorted order (by strategy)
 * Blocks are stored as a linked list
 * A block can be merged with its successor
 * Automatically cleans up dead and taken Tasks
 *
 * All write operations are only safe for the owner
 * Items can be read out by any thread
 *
 * Owner iterates through blocks backwards using back pointers (not safe for other threads)
 * Other threads iterate through blocks forward using next pointers
 * Blocks are sorted from largest to smallest. The local thread needs to add items to the smallest
 * block, and merge with larger, whereas it makes more sense for other threads to start with the largest block
 */
template <class Pheet, class Item>
class LSMLocalityTaskStorageBlock {
public:
	typedef LSMLocalityTaskStorageBlock<Pheet, Item> Self;

	typedef FrameMemoryManagerPlaceSingleton<Pheet> FrameManager;
	typedef typename FrameManager::Frame Frame;


	LSMLocalityTaskStorageBlock(size_t size)
	: frame_man(Pheet::template place_singleton<FrameManager>()),
	  filled(0), size(size), level(0), level_boundary(1), next(nullptr), prev(nullptr), in_use(false) {
		data = new std::atomic<Item*>[size];
		phases = new size_t[size];
	}
	~LSMLocalityTaskStorageBlock() {
		delete[] data;
		delete[] phases;
	}

	/*
	 * Assumes given item is already registered in frame if necessary
	 */
	bool try_put(Item* item, size_t p) {
		size_t f = filled.load(std::memory_order_relaxed);
		if(f == size) {
			return false;
		}
		else if(f == 0 ||
				item->strategy.prioritize(data[f-1].load(std::memory_order_relaxed)->strategy)) {
			data[f].store(item, std::memory_order_relaxed);
			phases[f] = p;
			// If new value for filled is seen, so is the item stored in it
			filled.store(f + 1, std::memory_order_release);

			if(f + 1 > (level_boundary)) {
				++level;
				level_boundary <<= 1;
			}
			return true;
		}
		return false;
	}

	/*
	 * Assumes given item is already registered in frame if necessary
	 */
	void put(Item* item, size_t p) {
		size_t f = filled.load(std::memory_order_relaxed);
		pheet_assert(f < size);
		pheet_assert(f == 0 ||
				item->strategy.prioritize(data[f-1].load(std::memory_order_relaxed)->strategy));
		data[f].store(item, std::memory_order_relaxed);
		phases[f] = p;
		// If new value for filled is seen, so is the item stored in it
		filled.store(f + 1, std::memory_order_release);

		if(f + 1 > (level_boundary)) {
			++level;
			level_boundary <<= 1;
		}
	}

	Item* top() {
		size_t f = filled.load(std::memory_order_relaxed);
		pheet_assert(f > 0);
		return data[f - 1].load(std::memory_order_relaxed);
	}

	Item* get_item(size_t pos) {
		pheet_assert(pos < size);
		return data[pos].load(std::memory_order_relaxed);
	}

	/*
	 * Scans the top tasks until either the block is empty or the top is not dead and not taken
	 */
	template <class Place>
	void pop_taken_and_dead(Place* local_place) {
		size_t f = filled.load(std::memory_order_relaxed);
		while(f > 0) {
			size_t f2 = f - 1;
			Item* item = data[f2].load(std::memory_order_relaxed);
			if(!item->taken.load(std::memory_order_relaxed)) {
				if(item->strategy.dead_task()) {
					// If we don't succeed someone else will, either way we won't execute the task
					item->take_and_delete();

					if(item->owner == local_place) {
						item->free_locally();
					}
					else {
						auto frame = item->frame.load(std::memory_order_relaxed);
						frame_man.rem_reg(frame, phases[f2]);
					}
				}
				else {
					break;
				}
			}
			else {
				if(item->owner == local_place) {
					item->free_locally();
				}
				else {
					auto frame = item->frame.load(std::memory_order_relaxed);
					frame_man.rem_reg(frame, phases[f2]);
				}
			}
			f = f2;
		}
		filled.store(f, std::memory_order_relaxed);

		if(f == 0) {
			level = 0;
			level_boundary = 1;
		}
		else {
			size_t next_boundary = level_boundary >> 1;
			while(f <= next_boundary) {
				pheet_assert(level > 0);
				--level;
				pheet_assert(next_boundary > 0);
				next_boundary >>= 1;
			}
			level_boundary = 1 << level;
		}
	}

	bool empty() const {
		size_t f = filled.load(std::memory_order_acquire);
		return f == 0;
	}

	size_t get_filled() const {
		return filled.load(std::memory_order_relaxed);
	}

	size_t acquire_filled() const {
		return filled.load(std::memory_order_acquire);
	}

	/*
	 * Not thread-safe. Only to be called by owner
	 */
	Self* get_prev() {
		return prev;
	}

	void set_prev(Self* prev) {
		this->prev = prev;
	}

	Self* get_next() {
		return next.load(std::memory_order_relaxed);
	}

	Self* acquire_next() {
		return next.load(std::memory_order_acquire);
	}

	void set_next(Self* next) {
		this->next.store(next, std::memory_order_relaxed);
	}

	void release_next(Self* next) {
		this->next.store(next, std::memory_order_release);
	}

	/*
	 * Merges the two given blocks into this block
	 * Assumes this block is empty
	 */
	template <class Place>
	void merge_into(Self* left, Self* right, Place* local_place) {
		pheet_assert(filled.load(std::memory_order_relaxed) == 0);
		pheet_assert(left->level <= right->level);
		pheet_assert(left->in_use);
		pheet_assert(right->in_use);

		size_t f = 0;
		size_t l_max = left->filled.load(std::memory_order_relaxed);
		size_t r_max = right->filled.load(std::memory_order_relaxed);
		size_t l = left->find_next_non_dead(0, local_place);
		size_t r = right->find_next_non_dead(0, local_place);

		while(l != l_max && r != r_max) {
			pheet_assert(l < l_max);
			pheet_assert(r < r_max);

			Item* l_item = left->data[l].load(std::memory_order_relaxed);
			Item* r_item = right->data[r].load(std::memory_order_relaxed);
			pheet_assert(l_item != nullptr);
			pheet_assert(r_item != nullptr);
			if(l_item == r_item) {
				// Get rid of doubles. Might not recognize doubles if there are different tasks
				// with exactly the same priority

				if(r_item->owner == local_place) {
					r_item->free_locally();
				}
				else {
					auto frame = r_item->frame.load(std::memory_order_relaxed);
					frame_man.rem_reg(frame, right->phases[r]);
				}
				r = right->find_next_non_dead(r + 1, local_place);
			}
			else if(l_item->strategy.prioritize(
					r_item->strategy)) {
				data[f].store(r_item, std::memory_order_relaxed);
				phases[f] = right->phases[r];
				r = right->find_next_non_dead(r + 1, local_place);
				++f;
			}
			else {
				data[f].store(l_item, std::memory_order_relaxed);
				phases[f] = left->phases[l];
				l = left->find_next_non_dead(l + 1, local_place);
				++f;
			}
		}

		if(l != l_max) {
			pheet_assert(r == r_max);
			do {
				pheet_assert(l < l_max);

				Item* l_item = left->data[l].load(std::memory_order_relaxed);
				pheet_assert(l_item != nullptr);
				data[f].store(l_item, std::memory_order_relaxed);
				phases[f] = left->phases[l];
				l = left->find_next_non_dead(l + 1, local_place);
				++f;
			} while(l != l_max);
		}
		else {
			while(r != r_max) {
				pheet_assert(r < r_max);

				Item* r_item = right->data[r].load(std::memory_order_relaxed);
				pheet_assert(r_item != nullptr);
				data[f].store(r_item, std::memory_order_relaxed);
				phases[f] = right->phases[r];
				r = right->find_next_non_dead(r + 1, local_place);
				++f;
			}
		}

		// No synchronization needed. Merged list is made visible using a release
		filled.store(f, std::memory_order_relaxed);

		pheet_assert(level == 0);
		pheet_assert(level_boundary == 1);
		while(f > (level_boundary)) {
			++level;
			level_boundary <<= 1;
		}
	}

	size_t get_level() {
		return level;
	}

	void reset() {
		// Make sure new list is visible when this list is encountered as empty
		filled.store(0, std::memory_order_release);

		level = 0;
		level_boundary = 1;
		// Make sure new list is visible when successors are not available any more
		next.store(nullptr, std::memory_order_release);
		prev = nullptr;
		in_use = false;
	}

	bool reusable() {
		pheet_assert(in_use || filled.load(std::memory_order_relaxed) == 0);
		return !in_use;
	}

	void mark_in_use() {
		in_use = true;
	}

private:
	/*
	 * Is used by merge_into to go through list.
	 * Should be called in a way so that each offset is processed exactly once, since some
	 * clean-up is performed on the way
	 */
	template <class Place>
	size_t find_next_non_dead(size_t offset, Place* local_place) {
		size_t f = filled.load(std::memory_order_relaxed);
		while(offset < f) {
			Item* item = data[offset].load(std::memory_order_relaxed);
			if(!item->taken.load(std::memory_order_relaxed)) {
				if(item->strategy.dead_task()) {
					// If we don't succeed someone else will, either way we won't execute the task
					item->take_and_delete();
				}
				else {
					break;
				}
			}

			// Items will never be touched again, so remove reference
			if(item->owner == local_place) {
				item->free_locally();
			}
			else {
				auto frame = item->frame.load(std::memory_order_relaxed);
				frame_man.rem_reg(frame, phases[offset]);
			}
			++offset;
		}
		return offset;
	}

	FrameManager& frame_man;

	std::atomic<Item*>* data;
	size_t* phases;
	std::atomic<size_t> filled;
	size_t size;
	size_t level;
	size_t level_boundary;

	std::atomic<Self*> next;
	Self* prev;

	bool in_use;
};

} /* namespace pheet */
#endif /* LSMLOCALITYTASKSTORAGEBLOCK_H_ */
