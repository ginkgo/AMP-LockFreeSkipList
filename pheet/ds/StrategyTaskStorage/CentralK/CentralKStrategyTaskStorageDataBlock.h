/*
 * CentralKStrategyTaskStorageDataBlock.h
 *
 *  Created on: 25.10.2012
 *      Author: mwimmer
 */

#ifndef CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_H_
#define CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_H_

#include "CentralKStrategyTaskStorageItem.h"
#include "CentralKStrategyTaskStorageDataBlockPerformanceCounters.h"
#include <pheet/misc/atomics.h>

namespace pheet {

template <class Pheet, class Place, typename TT, size_t BlockSize, size_t Tests>
class CentralKStrategyTaskStorageDataBlock {
public:
	typedef CentralKStrategyTaskStorageDataBlock<Pheet, Place, TT, BlockSize, Tests> Self;

	typedef TT T;

	typedef CentralKStrategyTaskStorageItem<Pheet, Place, TT> Item;
	typedef CentralKStrategyTaskStorageDataBlockPerformanceCounters<Pheet> PerformanceCounters;

	CentralKStrategyTaskStorageDataBlock()
	:offset(0), next(nullptr), active_threads(0), active(false) {
		for(size_t i = 0; i < BlockSize; ++i) {
			data[i] = nullptr;
		}
	}
	~CentralKStrategyTaskStorageDataBlock() {
	//	if(active_threads > 0) {
			// Last added block is never cleaned up. Clean it up on destruction
			// Cleanup happens at item level, otherwise we have a race condition
	//	}
	}

	bool put(size_t& cur_tail, Item* item, PerformanceCounters& pc) {
		// Take care not to break correct wraparounds when changing anything
		size_t k = item->strategy->get_k();
	//	size_t next_offset = offset + BlockSize;
	//	size_t old_tail = *tail;
	//	size_t cur_tail = old_tail;
		size_t array_offset = cur_tail - offset;

		while(array_offset < BlockSize) {
			pc.num_put_tests.incr();

			size_t cur_k = std::min(k, BlockSize - array_offset - 1);

			size_t to_add = Pheet::template rand_int<size_t>(cur_k);
			size_t i_limit = to_add + std::min(Tests, cur_k + 1);
			for(size_t i = to_add; i != i_limit; ++i) {
				size_t wrapped_i = i % (cur_k + 1);
				pheet_assert(array_offset + wrapped_i < BlockSize);
				if(data[array_offset + wrapped_i] == nullptr) {
					item->orig_position = cur_tail + wrapped_i;
					item->position = item->orig_position;
					if(PTR_CAS(&(data[array_offset + wrapped_i]), nullptr, item))
						return true;
				}
			}

			cur_tail += cur_k + 1;
			array_offset = cur_tail - offset;
		}
	//	update_tail(tail, old_tail, cur_tail);
		return false;
	}

	bool is_reusable() const {
		return !active;
	}

	void init_first(size_t num_threads) {
		pheet_assert(!active);
		this->active_threads = num_threads;
		this->active = true;
	}

	void deregister() {
		size_t old = SIZET_FETCH_AND_SUB(&active_threads, 1);
		pheet_assert(old > 0);
		if(old == 1) {
			pheet_assert(active_threads == 0);
			for(size_t i = 0; i < BlockSize; ++i) {
				if(data[i] != nullptr) {
					delete data[i]->strategy;
					data[i]->strategy = nullptr;
					data[i] = nullptr;
				}
			}
			next = nullptr;
			MEMORY_FENCE();
			active = false;
		}
	}

	void add_block(Self* block, size_t num_places) {
		pheet_assert(block->is_reusable());
		pheet_assert(block->next == nullptr);
		block->active_threads = num_places;
		block->active = true;

		Self* pred = this;
		block->offset = pred->offset + BlockSize;
		auto nextBlock = pred->next;
		while (nextBlock == nullptr)
		{
			if(PTR_CAS(&(pred->next), nullptr, block)) {
				pheet_assert(!pred->is_reusable());
				return;
			}
			nextBlock = pred->next;
		}

		// we failed to add the block, but some other thread must have succeeded -> make our block reusable again
		block->active = false;
	}

	bool in_block(size_t position) {
		return position - offset < BlockSize;
	}

	Item* get_item(size_t position) {
		pheet_assert(position - offset < BlockSize);
		return data[position - offset];
	}

	Self* get_next() {
		return next;
	}

	T take_rand_filled(size_t position, PerformanceCounters& pc, BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes>& num_unsuccessful_takes, BasicPerformanceCounter<Pheet, task_storage_count_successful_takes>& num_successful_takes) {
		// Take care not to break correct wraparounds when changing anything
		size_t off = offset;
		size_t array_offset = position - off;

		pheet_assert(array_offset < BlockSize);

		size_t max = BlockSize - array_offset - 1;

		size_t to_add = Pheet::template rand_int<size_t>(max);
		size_t i_limit = to_add + std::min(Tests, max + 1);

		for(size_t i = to_add; i != i_limit; ++i) {
			pc.num_take_tests.incr();
			size_t wrapped_i = i % (max + 1);
			size_t index = array_offset + wrapped_i;
			pheet_assert(index < BlockSize);

			Item* item = data[index];
			if(item != nullptr) {
				size_t g_index = off + index;
				if(item->position == g_index) {
					size_t k = item->strategy->get_k();
					// If k is smaller than the distance to position, tail must have been updated in the meantime.
					// We can't just take this item in this case or we might violate k-ordering
					if(wrapped_i <= k) {
						T ret = item->data;
						if(SIZET_CAS(&(item->position), g_index, g_index + (std::numeric_limits<size_t>::max() >> 1))) {
							num_successful_takes.incr();
							return ret;
						}
						else {
							num_unsuccessful_takes.incr();
						}
					}
				}
			}
		}
		return nullable_traits<T>::null_value;
	}
//	void verify(Item* item, size_t position);

	size_t get_offset() {
		return offset;
	}

private:
	Item* data[BlockSize];

	size_t offset;
	Self* next;

	size_t active_threads;
	bool active;
};

template <class DataBlock>
struct CentralKStrategyTaskStorageDataBlockReuseCheck {
	bool operator()(DataBlock const& block) const {
		return block.is_reusable();
	}
};

} /* namespace pheet */
#endif /* CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_H_ */
