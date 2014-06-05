/*
 * KLSMLocalityTaskStorage2Block.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGE2BLOCK_H_
#define KLSMLOCALITYTASKSTORAGE2BLOCK_H_

#include <atomic>
#include <algorithm>

#include "KLSMLocalityTaskStorage2BlockAnnouncement.h"

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
class KLSMLocalityTaskStorage2Block {
public:
	typedef KLSMLocalityTaskStorage2Block<Pheet, Item> Self;
	typedef KLSMLocalityTaskStorage2BlockAnnouncement<Pheet, Self> BlockAnnouncement;

	KLSMLocalityTaskStorage2Block(size_t size)
	: filled(0), size(size), k(std::numeric_limits<size_t>::max()), level(0), level_boundary(1), next(nullptr), prev(nullptr), in_use(false) {
		data = new std::atomic<Item*>[size];
	}
	~KLSMLocalityTaskStorage2Block() {
		delete[] data;
	}

	/*
	 * Assumes given item is already registered in frame if necessary
	 */
	bool try_put(Item* item) {
		size_t f = filled.load(std::memory_order_relaxed);
		if(f == size) {
			return false;
		}
		else if(f == 0 ||
				item->strategy.prioritize(data[f-1].load(std::memory_order_relaxed)->strategy)) {
			put(item);
			return true;
		}
		return false;
	}

	/*
	 * Assumes given item is already registered in frame if necessary
	 */
	void put(Item* item) {
		size_t f = filled.load(std::memory_order_relaxed);
		pheet_assert(f < size);
		pheet_assert(f == 0 ||
				item->strategy.prioritize(data[f-1].load(std::memory_order_relaxed)->strategy));
		data[f].store(item, std::memory_order_relaxed);
		// If new value for filled is seen, so is the item stored in it
		filled.store(f + 1, std::memory_order_release);
		k = std::min(k, item->strategy->get_k());

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

					item->free_locally();
				}
				else {
					break;
				}
			}
			else {
				item->free_locally();
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
		size_t k = std::numeric_limits<size_t>::max();
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
			// Duplicates can never occur since all items are local
			pheet_assert(l_item != r_item);
			if(l_item->strategy.prioritize(
					r_item->strategy)) {
				data[f].store(r_item, std::memory_order_relaxed);
				r = right->find_next_non_dead(r + 1, local_place);
				++f;
				k = std::min(k, r_item->strategy->get_k());
			}
			else {
				data[f].store(l_item, std::memory_order_relaxed);
				l = left->find_next_non_dead(l + 1, local_place);
				++f;
				k = std::min(k, l_item->strategy->get_k());
			}
		}

		if(l != l_max) {
			pheet_assert(r == r_max);
			do {
				pheet_assert(l < l_max);

				Item* l_item = left->data[l].load(std::memory_order_relaxed);
				pheet_assert(l_item != nullptr);
				data[f].store(l_item, std::memory_order_relaxed);
				l = left->find_next_non_dead(l + 1, local_place);
				++f;
				k = std::min(k, l_item->strategy->get_k());
			} while(l != l_max);
		}
		else {
			while(r != r_max) {
				pheet_assert(r < r_max);

				Item* r_item = right->data[r].load(std::memory_order_relaxed);
				pheet_assert(r_item != nullptr);
				data[f].store(r_item, std::memory_order_relaxed);
				r = right->find_next_non_dead(r + 1, local_place);
				++f;
				k = std::min(k, r_item->strategy->get_k());
			}
		}

		// No synchronization needed. Merged list is made visible using a release
		filled.store(f, std::memory_order_relaxed);
		this->k = k;

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
		k = std::numeric_limits<size_t>::max();

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

	BlockAnnouncement* get_last_announcement() {
		return ann;
	}

	void set_last_announcement(BlockAnnouncement* v) {
		ann = v;
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
			item->free_locally();
			++offset;
		}
		return offset;
	}

	std::atomic<Item*>* data;
	std::atomic<size_t> filled;
	size_t size;
	size_t k;
	size_t level;
	size_t level_boundary;

	std::atomic<Self*> next;
	Self* prev;

	bool in_use;

	BlockAnnouncement* ann;
};

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGE2BLOCK_H_ */
