/*
 * KLSMLocalityTaskStorageBlock.h
 *
 *  Created on: Sep 24, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGEBLOCK_H_
#define KLSMLOCALITYTASKSTORAGEBLOCK_H_

#include <atomic>
#include <algorithm>

#include <pheet/memory/Frame/FrameMemoryManagerPlaceSingleton.h>
#include "KLSMLocalityTaskStorageGlobalListItem.h"

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
class KLSMLocalityTaskStorageBlock {
public:
	typedef KLSMLocalityTaskStorageBlock<Pheet, Item> Self;
	typedef KLSMLocalityTaskStorageGlobalListItem<Pheet, Self> GlobalListItem;

	typedef FrameMemoryManagerPlaceSingleton<Pheet> FrameManager;
	typedef typename FrameManager::Frame Frame;

	KLSMLocalityTaskStorageBlock(size_t size, size_t max_level)
	: frame_man(Pheet::template place_singleton<FrameManager>()),
	  filled(0), owned_filled(0), size(size), max_level(max_level), level(0), level_boundary(1), next(nullptr),
	  prev(nullptr), k(std::numeric_limits<size_t>::max()),
	  in_use(false), global_list_item(nullptr) {
		data = new std::atomic<Item*>[size];
		phases = new size_t[size];
		owned_data = new std::atomic<Item*>[size];
	}
	~KLSMLocalityTaskStorageBlock() {
		delete[] data;
		delete[] phases;
		delete[] owned_data;
	}

	/*
	 * Assumes given item is already registered in frame if necessary
	 */
	bool try_put(Item* item, bool owned, size_t p) {
		size_t f = filled.load(std::memory_order_relaxed);
		pheet_assert(!is_global() || !owned);
		if(f == size) {
			return false;
		}
		else if(f == 0 ||
				item->strategy.prioritize(data[f-1].load(std::memory_order_relaxed)->strategy)) {
			data[f].store(item, std::memory_order_relaxed);
			phases[f] = p;
			// If new value for filled is seen, so is the item stored in it
			filled.store(f + 1, std::memory_order_release);
			if(owned) {
				size_t fo = owned_filled.load(std::memory_order_relaxed);
				owned_data[fo].store(item, std::memory_order_relaxed);
				owned_filled.store(fo + 1, std::memory_order_release);
			}

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
	void put(Item* item, bool owned, size_t p) {
		size_t f = filled.load(std::memory_order_relaxed);
		pheet_assert(!is_global());
		pheet_assert(f < size);
		pheet_assert(f == 0 ||
				item->strategy.prioritize(data[f-1].load(std::memory_order_relaxed)->strategy));
		data[f].store(item, std::memory_order_relaxed);
		phases[f] = p;
		// If new value for filled is seen, so is the item stored in it
		filled.store(f + 1, std::memory_order_release);
		if(owned) {
			size_t fo = owned_filled.load(std::memory_order_relaxed);
			owned_data[fo].store(item, std::memory_order_relaxed);
			owned_filled.store(fo + 1, std::memory_order_release);
			pheet_assert(k > 0);
			--k;
		}

		if(f + 1 > (level_boundary)) {
			++level;
			level_boundary <<= 1;
		}
	}

	/*
	 * Bookkeeping for k-relaxation
	 * Updated from outside
	 * (Since not all put operations add local items, and block
	 * does not have enough information to figure this out)
	 */
	void added_local_item(size_t item_k) {
		pheet_assert(!is_global());
		pheet_assert(!empty());
		if(item_k < k)
			k = item_k;
	}

	size_t get_k() {
		return k;
	}

	void mark_newly_global(GlobalListItem* gli) {
		pheet_assert(in_use);
		pheet_assert(global_list_item == nullptr);
		k = std::numeric_limits<size_t>::max();

		global_list_item = gli;
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

	Item* get_owned_item(size_t pos) {
		pheet_assert(pos < size);
		return owned_data[pos].load(std::memory_order_relaxed);
	}

	/*
	 * Scans the top tasks until either the block is empty or the top is not dead and not taken
	 */
	template <class Place>
	void pop_taken_and_dead(Place* local_place) {
		size_t f = filled.load(std::memory_order_relaxed);
		size_t fo = owned_filled.load(std::memory_order_relaxed);
		while(f > 0) {
			size_t f2 = f - 1;
			Item* item = data[f2].load(std::memory_order_relaxed);
			if(!item->taken.load(std::memory_order_relaxed)) {
				if(item->strategy.dead_task()) {
					// If we don't succeed someone else will, either way we won't execute the task
					item->take_and_delete();

					if(item->owner == local_place) {
						item->free_locally();

						pheet_assert(fo > 0);
						--fo;
						pheet_assert(owned_data[fo].load(std::memory_order_relaxed) == item);
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

					pheet_assert(fo > 0);
					--fo;
					pheet_assert(owned_data[fo].load(std::memory_order_relaxed) == item);
				}
				else {
					auto frame = item->frame.load(std::memory_order_relaxed);
					frame_man.rem_reg(frame, phases[f2]);
				}
			}
			f = f2;
		}
		filled.store(f, std::memory_order_relaxed);
		owned_filled.store(fo, std::memory_order_relaxed);

		if(f == 0) {
			level = 0;
			level_boundary = 1;
			k = std::numeric_limits<size_t>::max();
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

	size_t get_owned_filled() const {
		return owned_filled.load(std::memory_order_acquire);
	}

	size_t acquire_owned_filled() const {
		return owned_filled.load(std::memory_order_acquire);
	}

	/*
	 * Not thread-safe. Only to be called by owner
	 */
	Self* get_prev() {
		return prev;
	}

	void set_prev(Self* prev) {
		pheet_assert(prev == nullptr || prev->get_max_level() >= get_max_level() || prev->get_level() <= get_level());
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
	 * Only used for special cases to copy the content of a block into a smaller block
	 * Only the owner of both blocks may call this
	 */
	void copy_items(Self* other) {
		size_t f = other->filled.load(std::memory_order_relaxed);
		pheet_assert(f <= size);
		size_t fo = other->owned_filled.load(std::memory_order_relaxed);
		for(size_t i = 0; i < f; ++i) {
			data[i].store(other->data[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
			phases[i] = other->phases[i];
		}
		for(size_t i = 0; i < fo; ++i) {
			owned_data[i].store(other->owned_data[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
		}

		filled.store(f, std::memory_order_relaxed);
		owned_filled.store(fo, std::memory_order_relaxed);
		level = other->level;
		level_boundary = other->level_boundary;
		k = other->k;
	}

	/*
	 * Merges the two given blocks into this block
	 * Assumes this block is empty
	 */
	template <class Place>
	void merge_into(Self* left, Self* right, Place* local_place) {
		pheet_assert(filled.load(std::memory_order_relaxed) == 0);
		pheet_assert(left->in_use);
		pheet_assert(right->in_use);
		pheet_assert(!left->empty());
		pheet_assert(!right->empty());

		size_t min_k = std::numeric_limits<size_t>::max();
		size_t merged_local = 0;

		size_t f = 0;
		size_t fo = 0;
		size_t l_max = left->filled.load(std::memory_order_relaxed);
		size_t r_max = right->filled.load(std::memory_order_relaxed);
		size_t l = left->find_next_non_dead(0, local_place);
		size_t r = right->find_next_non_dead(0, local_place);

		bool global = is_global();

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

				if(l_item->owner == local_place) {
					l_item->free_locally();
				}
				else {
					auto frame = l_item->frame.load(std::memory_order_relaxed);
					frame_man.rem_reg(frame, left->phases[l]);
				}
				l = left->find_next_non_dead(l + 1, local_place);
			}
			else if(l_item->strategy.prioritize(
					r_item->strategy)) {
				if(r_item->owner == local_place) {
					if(!global) {
						++merged_local;
						size_t k = r_item->strategy.get_k();
						if(k < min_k) min_k = k;
					}

					owned_data[fo].store(r_item, std::memory_order_relaxed);
					++fo;
				}

				data[f].store(r_item, std::memory_order_relaxed);
				phases[f] = right->phases[r];
				r = right->find_next_non_dead(r + 1, local_place);
				++f;
			}
			else {
				if(l_item->owner == local_place) {
					if(!global) {
						++merged_local;
						size_t k = l_item->strategy.get_k();
						if(k < min_k) min_k = k;
					}

					owned_data[fo].store(l_item, std::memory_order_relaxed);
					++fo;
				}

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

				if(l_item->owner == local_place) {
					if(!global) {
						++merged_local;
						size_t k = l_item->strategy.get_k();
						if(k < min_k) min_k = k;
					}

					owned_data[fo].store(l_item, std::memory_order_relaxed);
					++fo;
				}

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

				if(r_item->owner == local_place) {
					if(!global) {
						++merged_local;
						size_t k = r_item->strategy.get_k();
						if(k < min_k) min_k = k;
					}

					owned_data[fo].store(r_item, std::memory_order_relaxed);
					++fo;
				}

				data[f].store(r_item, std::memory_order_relaxed);
				phases[f] = right->phases[r];
				r = right->find_next_non_dead(r + 1, local_place);
				++f;
			}
		}

		// No synchronization needed. Merged list is made visible using a release
		filled.store(f, std::memory_order_relaxed);
		owned_filled.store(fo, std::memory_order_relaxed);

		pheet_assert(level == 0);
		pheet_assert(level_boundary == 1);
		while(f > (level_boundary)) {
			++level;
			level_boundary <<= 1;
		}

		if(!global) {
			// Bookkeeping for k-relaxation
			// Since items are reordered we have to assume for each item that it is the oldest
			// Sometimes this will lead to worse values for k than for the previous lists
			// In this case we just use the values from the previous lists (combined in alternative_k)
			pheet_assert(left->k >= right->get_owned_filled());
			size_t alternative_k = std::min(left->k - right->get_owned_filled(), right->k);
			if(min_k < merged_local || alternative_k > (min_k - merged_local)) {
				k = alternative_k;
			} else {
				k = min_k - merged_local;
			}
		}
		else {
			k = std::numeric_limits<size_t>::max();
		}
	}

	size_t get_max_level() {
		return max_level;
	}

	size_t get_level() {
		return level;
	}

	void reset() {
		if(global_list_item != nullptr && (global_list_item->get_block() == this)) {
			global_list_item->update_block(nullptr);
		}
		global_list_item = nullptr;

		// Make sure new list is visible when this list is encountered as empty
		filled.store(0, std::memory_order_release);
		owned_filled.store(0, std::memory_order_release);

		level = 0;
		level_boundary = 1;
		// Make sure new list is visible when successors are not available any more
		next.store(nullptr, std::memory_order_release);
		prev = nullptr;
		in_use = false;

		k = std::numeric_limits<size_t>::max();
	}

	bool reusable() {
		pheet_assert(in_use || filled.load(std::memory_order_relaxed) == 0);
		pheet_assert(in_use || !is_global());
		return !in_use;
	}

	void mark_in_use() {
		in_use = true;
	}

	GlobalListItem* get_global_list_item() {
		if(global_list_item == nullptr || global_list_item->get_block() != this)
			return nullptr;
		return global_list_item;
	}
private:
	bool is_global() {
		return global_list_item != nullptr;
	}

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
	std::atomic<Item*>* owned_data;
	std::atomic<size_t> filled;
	std::atomic<size_t> owned_filled;
	size_t size;
	size_t max_level;
	size_t level;
	size_t level_boundary;

	std::atomic<Self*> next;
	Self* prev;

	size_t k;

	bool in_use;

	GlobalListItem* global_list_item;
};

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGEBLOCK_H_ */
