/*
 * DistKStrategyTaskStorageDataBlock.h
 *
 *  Created on: 15.01.2013
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef DISTKSTRATEGYTASKSTORAGEDATABLOCK_H_
#define DISTKSTRATEGYTASKSTORAGEDATABLOCK_H_

#include "DistKStrategyTaskStorageItem.h"
#include <pheet/misc/atomics.h>

namespace pheet {

template <class Pheet, class Place, typename TT, size_t BlockSize>
class DistKStrategyTaskStorageDataBlock {
public:
	typedef DistKStrategyTaskStorageDataBlock<Pheet, Place, TT, BlockSize> Self;

	typedef DistKStrategyTaskStorageItem<Pheet, Place, TT> Item;

	DistKStrategyTaskStorageDataBlock()
	:offset(0), prev(nullptr), next(nullptr), filled(0), active_items(BlockSize), locally_active_threads(0), state(4), active_threads(0) {
		for(size_t i = 0; i < BlockSize; ++i) {
			data[i] = nullptr;
		}
	}
	~DistKStrategyTaskStorageDataBlock() {
		clean_up_data();
	}

	void put(Item* item) {
		item->position = offset + filled;
		item->orig_position = offset + filled;
		data[filled] = item;

		// Make sure all changes to item are visible before item becomes visible
		MEMORY_FENCE();

		++filled;
		pheet_assert(filled <= BlockSize);
	}

	bool is_reusable() {
		pheet_assert(state != 4 || filled == 0);
		if(state == 1) {
			if(locally_active_threads == 0) {
				make_reusable();
			}
		}
		else if(state == 3) {
			if(active_threads == 0 && locally_active_threads == 0) {
//				pheet_assert(locally_active_threads == 0);
				make_reusable();
			}
		}

		pheet_assert(state != 4 || filled == 0);
		return state == 4;
				/*state == 3 ||
				(state == 2 &&
						(active_threads == 0 ||
								next->state == 3 ||
								next->local_id != local_id + 1));*/
	}

	bool is_empty() const {
		return active_items == 0;
	}

	bool is_full() const {
		return filled == BlockSize;
	}

	void init_global_first(size_t num_threads) {
		// An empty dummy block for the beginning
		state = 3;
		global_id = 0;
		active_items = 0;
		this->active_threads = num_threads;
//		this->target_active_threads = 0;
	}

	bool connect_list(Self* list) {
		size_t id = global_id;
		Self* block = list;

		do {
			++id;
			block->global_id = id;
			block = block->next;
		}while(block != nullptr);

		list->prev = this;
		return next == nullptr && PTR_CAS(&next, nullptr, list);
	}

	Item* get_item(size_t position) {
		pheet_assert(position < BlockSize);
		return data[position];
	}

	Self* get_next() {
		return next;
	}

	void set_next(Self* value) {
		pheet_assert(next == nullptr);
		value->reset(offset + BlockSize);
		value->prev = this;
		next = value;
	}

	void reset(size_t new_offset) {
		offset = new_offset;
		prev = nullptr;
		pheet_assert(next == nullptr);
		state = 0;
		active_items = BlockSize;
	}

	void set_offset(size_t value) {
		offset = value;
	}

	size_t get_offset() {
		return offset;
	}

	uint8_t get_state() {
		return state;
	}

	void set_state(uint8_t value) {
		state = value;
	}

	void set_active_threads(size_t value) {
		pheet_assert(active_threads == 0);
		active_threads = value;
	}

	size_t get_filled() {
		return filled;
	}

	void mark_item_used() {
		--active_items;
	}

	void perform_cleanup_check() {
		if(active_items == 0) {
			pheet_assert(state != 4);
			if(state == 0 && next != nullptr) { // If next == nullptr, another item is guaranteed to be added later
				state = 1;
				if(prev != nullptr) {
					pheet_assert(prev->next == this);
					prev->next = next;
				}
				next->prev = prev;
				MEMORY_FENCE();
				if(locally_active_threads == 0) {
					make_reusable();
				}
			}
			else if(state == 2) {
				if(active_threads == 0 && locally_active_threads == 0) {
					make_reusable();
				}
				else {
					state = 3;
				}
			}
		}
	}

	void mark_processed_globally() {
		SIZET_ATOMIC_SUB(&active_threads, 1);
		pheet_assert(active_threads > 0 || next != nullptr);
	}

	void register_locally() {
		SIZET_ATOMIC_ADD(&locally_active_threads, 1);
	}

	void deregister_locally() {
		pheet_assert(locally_active_threads > 0);
		SIZET_ATOMIC_SUB(&locally_active_threads, 1);
	}

	void mark_block_final() {
		// Rest of block will never be filled. Adapt active items accordingly
		pheet_assert(filled <= BlockSize);
		active_items -= BlockSize - filled;
	}
private:
	void global_unlink() {

	}

	void make_reusable() {
		clean_up_data();

		next = nullptr;
		filled = 0;
		pheet_assert(active_items == 0);
		state = 4;
	}

	void clean_up_data() {
		pheet_assert(active_items == 0 || (next == nullptr && active_items == BlockSize - filled));

		for(size_t i = 0; i < filled; ++i) {
			pheet_assert(data[i] != nullptr);

			pheet_assert(data[i]->position != data[i]->orig_position);
			delete data[i]->strategy;
			data[i]->strategy = nullptr;
		}
	}

	Item* data[BlockSize];

	size_t offset;
	Self* prev;
	Self* next;

	size_t filled;
	size_t active_items;

	// Atomically incremented by other threads when usable item is found. Decremented after last item has been copied. Necessary for memory management of strategies
	// Cleanup may be performed safely if value = 0 AFTER active_items was set to 0
	size_t locally_active_threads;

	// 0: local, 1: locally unlinked, 2: global, 3: global unlinked, 4: reusable
	// local blocks can be unlinked and reused immediatly without caring about anything. No guarantees given to stealers
	// global blocks have to be unlinked in the logarithmic way (both safety against conflicts, and
	// If it's global and unlinked, but active_threads != 0 (and > prev->active_threads) wait until == prev->active_threads (needs to be stored, since prev pointer is set to null) or next is reusable (or was reused)
	uint8_t state;

	// Only relevant if used globally
	size_t global_id;

	size_t active_threads;
	// Target to be reached for block to be reused safely
//	size_t target_active_threads;
};

template <class DataBlock>
struct DistKStrategyTaskStorageDataBlockReuseCheck {
	bool operator()(DataBlock&  block) {
		return block.is_reusable();
	}
};

} /* namespace pheet */
#endif /* DISTKSTRATEGYTASKSTORAGEDATABLOCK_H_ */
