/*
 * BasicLinkedListStrategyTaskStorage.h
 *
 *  Created on: 06.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICLINKEDLISTSTRATEGYTASKSTORAGE_H_
#define BASICLINKEDLISTSTRATEGYTASKSTORAGE_H_

#include "BasicLinkedListStrategyTaskStoragePerformanceCounters.h"
#include "BasicLinkedListStrategyTaskStorageDataBlock.h"
#include "BasicLinkedListStrategyTaskStorageStream.h"
#include "../../StrategyHeap/Basic/BasicStrategyHeap.h"
#include "../../StrategyHeap/Merge/MergeStrategyHeap.h"
//#include "../../StrategyHeap/Volatile2/VolatileStrategyHeap2.h"
#include "../../../misc/type_traits.h"
#include <pheet/memory/ItemReuse/ItemReuseMemoryManager.h>
#include <pheet/ds/PriorityQueue/FibonacciSame/FibonacciSameHeap.h>

#include <vector>
#include <queue>
#include <iostream>

namespace pheet {

template <class Pheet, typename TT, class TaskStorage, class Stream, class StealerRef>
struct BasicLinkedListStrategyTaskStorageItem {
	typedef TT Item;

	typename Pheet::Scheduler::BaseStrategy* strategy;
	TT item;
	size_t taken;
	void (Stream::*stealer_push)(StealerRef& stealer);
	void (Stream::*task_storage_push)(TaskStorage& task_storage, typename Pheet::Scheduler::BaseStrategy* strategy, TT const& stream_ref);
};

template <class Pheet, typename DataBlock>
struct BasicLinkedListStrategyTaskStorageLocalReference {
	DataBlock* block;
	size_t index;
};

template <class Pheet, class TaskStorage>
class BasicLinkedListStrategyTaskStorageStrategyRetriever {
public:
	typedef BasicLinkedListStrategyTaskStorageStrategyRetriever<Pheet, TaskStorage> Self;
	typedef typename TaskStorage::LocalRef Item;

	BasicLinkedListStrategyTaskStorageStrategyRetriever(TaskStorage* task_storage)
	:task_storage(task_storage) {}

	BasicLinkedListStrategyTaskStorageStrategyRetriever(BasicLinkedListStrategyTaskStorageStrategyRetriever const& other)
	:task_storage(other.task_storage) {}

	BasicLinkedListStrategyTaskStorageStrategyRetriever(BasicLinkedListStrategyTaskStorageStrategyRetriever&& other)
	:task_storage(other.task_storage) {}

	typename Pheet::Scheduler::BaseStrategy* operator()(Item const& item) {
		return (item.block->get_data(item.index).strategy);
	}

	template<class Strategy>
	inline bool is_active(Item& item) {
		return (item.block->get_data(item.index).taken & 1) == 0  && !reinterpret_cast<Strategy*>(item.block->get_data(item.index).strategy)->dead_task();
	}
/*
	inline void mark_removed(Item& item) {
		item.block->mark_removed(item.index, task_storage);
	}

	inline bool clean_item(Item& item) {
		if(!is_active(item)) {
			mark_removed(item);
			return true;
		}
		return false;
	}*/

private:
	TaskStorage* task_storage;
};

template <class DataBlock>
struct BasicLinkedListStrategyTaskStorageDataBlockReuseCheck {
	bool operator()(DataBlock& b) {
		return b.is_reusable();
	}
};

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
class BasicLinkedListStrategyTaskStorageImpl {
public:
	typedef BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize> Self;

	typedef TT T;

	template <class UnassignedStealer>
	using StreamT = BasicLinkedListStrategyTaskStorageStream<Pheet, Self, UnassignedStealer>;

	template <class UnassignedStream, class UnassignedStealerRef>
	using ItemT = BasicLinkedListStrategyTaskStorageItem<Pheet, T, Self, UnassignedStream, UnassignedStealerRef>;

	template <class UnassignedItem>
	using DataBlockT = BasicLinkedListStrategyTaskStorageDataBlock<Pheet, UnassignedItem, Self, BlockSize>;

	typedef StealerT<Pheet, Self> Stealer;
	typedef typename Stealer::StealerRef StealerRef;

	typedef StreamT<StealerRef> Stream;

	typedef ItemT<Stream, StealerRef> Item;
	typedef DataBlockT<Item> DataBlock;

	typedef BasicLinkedListStrategyTaskStorageLocalReference<Pheet, DataBlock> LocalRef;
	typedef BasicLinkedListStrategyTaskStorageStrategyRetriever<Pheet, Self> StrategyRetriever;

	typedef StrategyHeapT<Pheet, LocalRef, StrategyRetriever> StrategyHeap;
	typedef BasicLinkedListStrategyTaskStoragePerformanceCounters<Pheet, typename StrategyHeap::PerformanceCounters>
		PerformanceCounters;

	typedef ItemReuseMemoryManager<Pheet, DataBlock, BasicLinkedListStrategyTaskStorageDataBlockReuseCheck<DataBlock> > DataBlockMemoryManager;

	template <template <class, typename, class> class NewStrategyHeap>
	using WithStrategyHeap = BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, NewStrategyHeap, BlockSize>;

	template <class NewPheet, typename NewTT>
	using BT = BasicLinkedListStrategyTaskStorageImpl<NewPheet, NewTT, StealerT, StrategyHeapT, BlockSize>;

	BasicLinkedListStrategyTaskStorageImpl();
	BasicLinkedListStrategyTaskStorageImpl(PerformanceCounters& pc);
	~BasicLinkedListStrategyTaskStorageImpl();

	template <class Strategy>
	void push(Strategy&& s, T item);
	T pop();
	void make_empty();
	T& peek();

	inline size_t size() const { return get_length(); }
	size_t get_length() const { return heap.size(); }
	inline bool empty() const { return is_empty(); }
	bool is_empty() const { return heap.is_empty(); }
	inline bool is_full() const { return false; }

	inline DataBlock* const& get_front_block() { return front; }
	inline DataBlock* create_block() {
		return &(blocks.acquire_item());
	}

	static void print_name() {
		std::cout << "BasicLinkedListStrategyTaskStorage";
	}
private:

	void update_front();

	DataBlockMemoryManager blocks;

	DataBlock* front;
	DataBlock* back;
	size_t back_index;

	PerformanceCounters pc;
	StrategyHeap heap;
//	std::vector<DataBlock*> data_block_reuse;
};

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize>::BasicLinkedListStrategyTaskStorageImpl()
:heap(StrategyRetriever(this), pc.strategy_heap_performance_counters){
	front = &(blocks.acquire_item());
	back = front;
//	front->reuse(0, nullptr);
	front->add_predecessor();
}

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize>::BasicLinkedListStrategyTaskStorageImpl(PerformanceCounters& pc)
:pc(pc), heap(StrategyRetriever(this), this->pc.strategy_heap_performance_counters){
	front = &(blocks.acquire_item());
	back = front;
//	front->reuse(0, nullptr);
	front->add_predecessor();
}

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize>::~BasicLinkedListStrategyTaskStorageImpl() {
	pheet_assert(heap.is_empty());

//	pheet_assert(current_view->get_front() == back);
//	pheet_assert(back->is_first());

	// Views are automatically deleted by the memory manager

/*	DataBlock* tmp = front;

	while (tmp != back) {
		DataBlock* next = tmp->get_next();
		delete tmp;
		tmp = next;
	}
	delete tmp;*/
}

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
template <class Strategy>
void BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize>::push(Strategy&& s, T item) {
	Item it;
	it.strategy = new Strategy(std::move(s));
	it.item = item;
	it.taken = back->get_taken_offset();
	it.stealer_push = &Stream::template stealer_push_ref<Strategy>;
	it.task_storage_push = &Stream::template task_storage_push<Strategy>;

	LocalRef r;
	r.block = back;
	r.index = back->push(std::move(it), this);
	heap.template push<Strategy>(std::move(r));

	if(back->get_next() != nullptr) {
		back = back->get_next();
	}
}

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
TT BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize>::pop() {
	while(heap.size() > 0) {
		LocalRef r = heap.pop();

		T ret;
		if(r.block->local_take(r.index, ret, this)) {
			update_front();
			return ret;
		}
	}
	update_front();
	return nullable_traits<T>::null_value;
}

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
void BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize>::make_empty() {
	while(!heap.is_empty()) {
		LocalRef r = heap.pop();

		pheet_assert(r.block->is_taken(r.index));
		r.block->mark_removed(r.index, this);
	}
	update_front();
}

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
TT& BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize>::peek() {
	auto r = heap.peek();
	return r.block->peek(r.index);
}

template <class Pheet, typename TT, template <class, class> class StealerT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize>
void BasicLinkedListStrategyTaskStorageImpl<Pheet, TT, StealerT, StrategyHeapT, BlockSize>::update_front() {
	while(!front->is_active()) {
		front->sub_predecessor();
		front = front->get_next();
		front->add_predecessor();
	}
}

template <class Pheet, typename T, template <class, class> class StealerT>
using BasicLinkedListStrategyTaskStorage = BasicLinkedListStrategyTaskStorageImpl<Pheet, T, StealerT, BasicStrategyHeap, 256>;

template <class Pheet, typename T, template <class, class> class StealerT>
using BasicLinkedListStrategyTaskStorageMergeHeap = BasicLinkedListStrategyTaskStorageImpl<Pheet, T, StealerT, MergeStrategyHeap, 256>;

}

#endif /* BASICLINKEDLISTSTRATEGYTASKSTORAGE_H_ */
