/*
 * CentralKStrategyTaskStoragePlace.h
 *
 *  Created on: 03.10.2013
 *      Author: mwimmer, mpoeter
 */

#ifndef CENTRALKSTRATEGYTASKSTORAGEPLACE_CPP11_H_
#define CENTRALKSTRATEGYTASKSTORAGEPLACE_CPP11_H_

#include "CentralKStrategyTaskStorageDataBlock.h"
#include "CentralKStrategyTaskStorageItem.h"
#include "CentralKStrategyTaskStoragePerformanceCounters.h"

#include <atomic>
#include <pheet/memory/BlockItemReuse/BlockItemReuseMemoryManager.h>

#include "../../../misc/atomics_disable_macros.h"

namespace pheet { namespace cpp11 {

template <class Pheet, class Item>
struct CentralKStrategyTaskStorageItemReference {
	Item* item;
	size_t position;
	// If strategy equals item->strategy => locally created
	typename Pheet::Scheduler::BaseStrategy* strategy;
};

template <class Pheet, class Ref>
class CentralKStrategyTaskStorageStrategyRetriever {
public:
	typedef CentralKStrategyTaskStorageStrategyRetriever<Pheet, Ref> Self;

	CentralKStrategyTaskStorageStrategyRetriever() {}

	typename Pheet::Scheduler::BaseStrategy* operator()(Ref const& ref) {
		return ref.strategy;
	}

	template<class Strategy>
	inline bool is_active(Ref const& ref) {
		return ref.item->position.load(std::memory_order_relaxed) == ref.position && !reinterpret_cast<Strategy*>(ref.strategy)->dead_task();
	}
};

template <class Pheet, class TaskStorage, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize, size_t Tests, bool LocalKPrio>
class CentralKStrategyTaskStoragePlace {
public:
	typedef CentralKStrategyTaskStoragePlace<Pheet, TaskStorage, TT, StrategyHeapT, BlockSize, Tests, LocalKPrio> Self;

	typedef CentralKStrategyTaskStorageDataBlock<Pheet, Self, TT, BlockSize, Tests> DataBlock;

	typedef TT T;
	typedef CentralKStrategyTaskStorageItem<Pheet, Self, TT> Item;
	typedef CentralKStrategyTaskStorageItemReference<Pheet, Item> Ref;
	typedef CentralKStrategyTaskStorageStrategyRetriever<Pheet, Ref> StrategyRetriever;

	typedef StrategyHeapT<Pheet, Ref, StrategyRetriever> StrategyHeap;
	typedef CentralKStrategyTaskStoragePerformanceCounters<Pheet, typename StrategyHeap::PerformanceCounters>
			PerformanceCounters;

	typedef BlockItemReuseMemoryManager<Pheet, Item, CentralKStrategyTaskStorageItemReuseCheck<Item> > ItemMemoryManager;
	typedef BlockItemReuseMemoryManager<Pheet, DataBlock, CentralKStrategyTaskStorageDataBlockReuseCheck<DataBlock> > DataBlockMemoryManager;

	typedef typename Pheet::Scheduler::Place SchedulerPlace;

	CentralKStrategyTaskStoragePlace(TaskStorage* task_storage, SchedulerPlace*, PerformanceCounters pc):
		pc(pc),
		task_storage(task_storage),
		heap(sr, pc.strategy_heap_performance_counters),
		head(0)
	{
		DataBlock* tmp = task_storage->start_block;
		if (tmp == nullptr)
		{
			// This assumes the first place is initialized before all others, otherwise synchronization would be needed!
			tmp = &(data_blocks.acquire_item());
			tmp->init_first(task_storage->get_num_places());
			task_storage->start_block = tmp;
		}
		tail_block = tmp;
		head_block = tmp;
	}

	~CentralKStrategyTaskStoragePlace() {
		// clean_up needs to be called by scheduler before, so we can assume no
		// old references are stored in priority queue
		pheet_assert(heap.empty());
	}

	/**
	 * Needs to be called by scheduling system before scheduler terminates
	 */
	void clean_up()
	{
		while (!heap.empty())
		{
			Ref r = heap.pop();
			// All items should have been processed
			pheet_assert(r.position != r.item->position.load(std::memory_order_relaxed));
			delete r.strategy;
			r.strategy = nullptr;
		}
	}

	template <class Strategy>
	void push(Strategy&& s, T data)
	{
		Item& it = items.acquire_item();
		pheet_assert(it.strategy == nullptr);
		it.strategy = new Strategy(s);
		it.data = data;
		it.item_push = &Self::template item_push<Strategy>;
		it.owner = this;

		size_t old_tail = task_storage->tail.load(std::memory_order_relaxed);
		size_t cur_tail = old_tail;

		while (!tail_block->put(cur_tail, &it, pc.data_block_performance_counters))
		{
			if (tail_block->get_next(std::memory_order_relaxed) == nullptr)
			{
				DataBlock& next_block = data_blocks.acquire_item();
				pc.num_blocks_created.incr();
				tail_block->add_block(&next_block, task_storage->get_num_places());
			}
			pheet_assert(tail_block->get_next(std::memory_order_relaxed) != nullptr);
			pheet_assert(tail_block->get_offset() + BlockSize == tail_block->get_next(std::memory_order_relaxed)->get_offset());
			tail_block = tail_block->get_next(std::memory_order_acquire);
		}

		if (cur_tail != old_tail)
		{
			size_t nold_tail = task_storage->tail.load(std::memory_order_relaxed);
			ptrdiff_t diff = static_cast<ptrdiff_t>(cur_tail) - static_cast<ptrdiff_t>(nold_tail);
			while (diff > 0)
			{
				if (task_storage->tail.compare_exchange_weak(nold_tail, cur_tail, std::memory_order_release, std::memory_order_relaxed))
					break;
				diff = static_cast<ptrdiff_t>(cur_tail) - static_cast<ptrdiff_t>(nold_tail);
			}
		}

		Ref r;
		r.item = &it;
		r.position = it.orig_position;
		r.strategy = new Strategy(std::move(s));

		heap.template push<Strategy>(std::move(r));
	}

	T pop()
	{
		update_heap();

		while (heap.size() > 0)
		{
			Ref r = heap.pop();

			pheet_assert(r.strategy != nullptr);
			delete r.strategy;

			if (r.item->position.load(std::memory_order_relaxed) == r.position)
			{
				T ret = r.item->data;
				if (r.item->position.compare_exchange_weak(r.position, r.position + (std::numeric_limits<size_t>::max() >> 1), std::memory_order_relaxed, std::memory_order_relaxed))
				{
					pc.num_successful_takes.incr();
					return ret;
				}
				else
				{
					pc.num_unsuccessful_takes.incr();
					pc.num_taken_heap_items.incr();
				}
			}
			else
				pc.num_taken_heap_items.incr();

			update_heap();
		}

		// Heap is empty. Try getting random item directly from fifo queue (without heap)

		// Tries to find and take a random item from the queue inside the block
		// Synchronization and verification all take place inside this method.
		// Returned item is free to use by this thread
		return head_block->take_rand_filled(head, pc.data_block_performance_counters, pc.num_unsuccessful_takes, pc.num_successful_takes);
	}

	template <class Strategy>
	void item_push(Item* item, size_t position)
	{
		if (reinterpret_cast<Strategy*>(item->strategy)->dead_task())
			return;
		
		Ref r;
		r.item = item;
		r.position = position;
		// TODO: check whether the position is still valid, otherwise end function

		Strategy* s = new Strategy(*reinterpret_cast<Strategy*>(item->strategy));
		s->rebase();
		r.strategy = s;

		heap.template push<Strategy>(std::move(r));
	}

	bool is_full() const {
		return false;
	}

	TaskStorage* get_central_task_storage() {
		return task_storage;
	}

	size_t size() const {
		return heap.size();
	}
private:
	void update_heap()
	{
		// Check whether update is necessary
		if (!heap.empty() && task_storage->tail.load(std::memory_order_relaxed) == head)
			return;

		process_until(task_storage->tail.load(std::memory_order_acquire));
	}

	void process_until(size_t limit)
	{
		while (head != limit)
		{
			deregister_old_blocks();

			Item* item = head_block->get_item(head);
			if (item != nullptr && item->owner != this && item->position.load(std::memory_order_relaxed) == head)
			{
				// Push item to local heap
				auto ip = item->item_push;
				(this->*ip)(item, head);
			}

			++head;
		}
		deregister_old_blocks();
	}

	void deregister_old_blocks()
	{
		while (!head_block->in_block(head))
		{
			pheet_assert(head_block->get_next(std::memory_order_relaxed) != nullptr);
			DataBlock* next = head_block->get_next(std::memory_order_acquire);
			if (head_block == tail_block) // Make sure tail block doesn't lag behind
				tail_block = next;

			pheet_assert(next != nullptr);
			pheet_assert(static_cast<ptrdiff_t>(tail_block->get_offset()) - static_cast<ptrdiff_t>(head_block->get_offset()) >= 0);
			head_block->deregister();
			head_block = next;
		}
	}

	PerformanceCounters pc;

	TaskStorage* task_storage;
	StrategyRetriever sr;
	StrategyHeap heap;

	DataBlock* tail_block;
	DataBlock* head_block;
	size_t head;

	ItemMemoryManager items;
	DataBlockMemoryManager data_blocks;
};

} // namespace cpp11
} // namespace pheet
#endif /* CENTRALKSTRATEGYTASKSTORAGEPLACE_CPP11_H_ */
