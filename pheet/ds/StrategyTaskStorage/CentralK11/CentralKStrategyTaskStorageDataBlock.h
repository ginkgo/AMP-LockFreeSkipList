/*
 * CentralKStrategyTaskStorageDataBlock.h
 *
 *  Created on: 03.10.2013
 *      Author: mwimmer, mpoeter
 */

#ifndef CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_CPP11_H_
#define CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_CPP11_H_

#include "CentralKStrategyTaskStorageItem.h"
#include "CentralKStrategyTaskStorageDataBlockPerformanceCounters.h"

#include "../../../misc/atomics_disable_macros.h"

namespace pheet { namespace cpp11 {

template <class Pheet, class Place, typename TT, size_t BlockSize, size_t Tests>
class CentralKStrategyTaskStorageDataBlock {
public:
	typedef CentralKStrategyTaskStorageDataBlock<Pheet, Place, TT, BlockSize, Tests> Self;

	typedef TT T;

	typedef CentralKStrategyTaskStorageItem<Pheet, Place, TT> Item;
	typedef CentralKStrategyTaskStorageDataBlockPerformanceCounters<Pheet> PerformanceCounters;

	CentralKStrategyTaskStorageDataBlock():
		offset(0),
		next(nullptr),
		active_threads(0),
		active(false)
	{
		for (auto& elem : data)
			elem.store(nullptr, std::memory_order_relaxed);
	}

	~CentralKStrategyTaskStorageDataBlock() {
	//	if (active_threads > 0) {
			// Last added block is never cleaned up. Clean it up on destruction
			// Cleanup happens at item level, otherwise we have a race condition
	//	}
	}

	bool put(size_t& cur_tail, Item* item, PerformanceCounters& pc)
	{
		// Take care not to break correct wraparounds when changing anything
		const size_t k = item->strategy->get_k();
		size_t array_offset = cur_tail - offset;

		while (array_offset < BlockSize)
		{
			pc.num_put_tests.incr();

			const size_t cur_k = std::min(k, BlockSize - array_offset - 1);

			const size_t to_add = Pheet::template rand_int<size_t>(cur_k);
			const size_t i_limit = to_add + std::min(Tests, cur_k + 1);
			for (size_t i = to_add; i != i_limit; ++i)
			{
				const size_t wrapped_i = i % (cur_k + 1);
				pheet_assert(array_offset + wrapped_i < BlockSize);				
				auto& elem = data[array_offset + wrapped_i];
				if (elem.load(std::memory_order_relaxed) == nullptr)
				{
					auto position = cur_tail + wrapped_i;
					item->orig_position = position;
					item->position.store(position, std::memory_order_relaxed);
					Item* null = nullptr;
					if (elem.compare_exchange_strong(null, item, std::memory_order_release, std::memory_order_relaxed))
						return true;
				}
			}

			cur_tail += cur_k + 1;
			array_offset = cur_tail - offset;
		}
		return false;
	}

	bool is_reusable() const
	{
		return !active.load(std::memory_order_relaxed);
	}

	void init_first(size_t num_threads)
	{
		pheet_assert(!active.load(std::memory_order_relaxed));
		this->active_threads.store(num_threads, std::memory_order_relaxed);
		this->active.store(true, std::memory_order_relaxed);
	}

	void deregister()
	{
		size_t old = active_threads.fetch_sub(1, std::memory_order_relaxed);
		pheet_assert(old > 0);
		if (old == 1)
		{
			pheet_assert(active_threads.load(std::memory_order_relaxed) == 0);
			for (auto& elem : data)
			{
				auto item = elem.load(std::memory_order_relaxed);
				if (item != nullptr)
				{
					delete item->strategy;
					item->strategy = nullptr;
					elem.store(nullptr, std::memory_order_relaxed);
				}
			}
			next.store(nullptr, std::memory_order_relaxed);
			active.store(false, std::memory_order_release);
		}
	}

	void add_block(Self* block, size_t num_places)
	{
		// establish synchronize-with relationship with active.store from deregister()
		block->active.load(std::memory_order_acquire);

		pheet_assert(block->is_reusable());
		pheet_assert(block->next.load(std::memory_order_relaxed) == nullptr);
		block->active_threads.store(num_places, std::memory_order_relaxed);
		block->active.store(true, std::memory_order_relaxed);

		Self* pred = this;
		block->offset = pred->offset + BlockSize;
		auto nextBlock = pred->next.load(std::memory_order_relaxed);
		while (nextBlock == nullptr)
		{
			if (pred->next.compare_exchange_weak(nextBlock, block, std::memory_order_release, std::memory_order_relaxed))
			{
				pheet_assert(!pred->is_reusable());
				return;
			}
		}

		// we failed to add the block, but some other thread must have succeeded -> make our block reusable again
		block->active.store(false, std::memory_order_relaxed);
	}

	bool in_block(size_t position) {
		return position - offset < BlockSize;
	}

	Item* get_item(size_t position) {
		pheet_assert(position - offset < BlockSize);
		return data[position - offset].load(std::memory_order_acquire);
	}

	Self* get_next(std::memory_order order) {
		return next.load(order);
	}

	T take_rand_filled(size_t position, PerformanceCounters& pc, BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes>& num_unsuccessful_takes, BasicPerformanceCounter<Pheet, task_storage_count_successful_takes>& num_successful_takes)
	{
		// Take care not to break correct wraparounds when changing anything
		size_t off = offset;
		size_t array_offset = position - off;

		pheet_assert(array_offset < BlockSize);

		size_t max = BlockSize - array_offset - 1;

		size_t to_add = Pheet::template rand_int<size_t>(max);
		size_t i_limit = to_add + std::min(Tests, max + 1);

		for (size_t i = to_add; i != i_limit; ++i)
		{
			pc.num_take_tests.incr();
			size_t wrapped_i = i % (max + 1);
			size_t index = array_offset + wrapped_i;
			pheet_assert(index < BlockSize);

			Item* item = data[index].load(std::memory_order_acquire);
			if (item != nullptr)
			{
				size_t g_index = off + index;
				if (item->position.load(std::memory_order_relaxed) == g_index)
				{
					size_t k = item->strategy->get_k();
					// If k is smaller than the distance to position, tail must have been updated in the meantime.
					// We can't just take this item in this case or we might violate k-ordering
					if (wrapped_i <= k)
					{
						T ret = item->data;
						if (item->position.compare_exchange_weak(g_index, g_index + (std::numeric_limits<size_t>::max() >> 1), std::memory_order_relaxed, std::memory_order_relaxed))
						{
							num_successful_takes.incr();
							return ret;
						}
						else
							num_unsuccessful_takes.incr();
					}
				}
			}
		}
		return nullable_traits<T>::null_value;
	}

	size_t get_offset() {
		return offset;
	}

private:
	std::atomic<Item*> data[BlockSize];

	size_t offset;
	std::atomic<Self*> next;

	std::atomic<size_t> active_threads;
	std::atomic<bool> active;
};

template <class DataBlock>
struct CentralKStrategyTaskStorageDataBlockReuseCheck {
	bool operator()(DataBlock const& block) const {
		return block.is_reusable();
	}
};

} // namespace cpp11
} // namespace pheet

#endif /* CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_CPP11_H_ */
