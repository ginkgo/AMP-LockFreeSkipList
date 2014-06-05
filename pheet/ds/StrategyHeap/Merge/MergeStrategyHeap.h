/*
 * MergeStrategyHeap.h
 *
 *  Created on: Mar 07, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef MERGESTRATEGYHEAP_H_
#define MERGESTRATEGYHEAP_H_

#include <map>
#include <typeindex>
#include <vector>
#include <ctime>

#include "MergeStrategyItemHeap.h"
#include "MergeStrategyHeapHeap.h"
#include "MergeStrategyHeapPerformanceCounters.h"

namespace pheet {


template <class Pheet, typename TT, class StrategyRetriever>
class MergeStrategyHeap {
public:
	typedef TT T;
	typedef MergeStrategyHeapBase<Pheet, T> HeapBase;
	typedef typename Pheet::Scheduler::BaseStrategy BaseStrategy;
	typedef MergeStrategyHeapPerformanceCounters<Pheet> PerformanceCounters;

	MergeStrategyHeap(StrategyRetriever sr, PerformanceCounters& pc);
	~MergeStrategyHeap();

	template <class Strategy>
	void push(T const& item);

	T pop();
	T& peek();

	bool empty() const { return is_empty(); }
	bool is_empty() const;
	size_t size() const { return total_size; }

	static void print_name();

private:
	std::map<std::type_index, HeapBase*> item_heaps;
	std::map<std::type_index, HeapBase*> heap_heaps;
	StrategyRetriever sr;
	MergeStrategyHeapHeap<Pheet, TT, BaseStrategy, BaseStrategy, StrategyRetriever> root_heap;
	PerformanceCounters pc;

	size_t total_size;
};

template <class Pheet, typename TT, class StrategyRetriever>
MergeStrategyHeap<Pheet, TT, StrategyRetriever>::MergeStrategyHeap(StrategyRetriever sr, PerformanceCounters& pc)
:sr(std::move(sr)), root_heap(sr, heap_heaps), pc(pc), total_size(0) {
	heap_heaps[std::type_index(typeid(BaseStrategy))] = &root_heap;
}

template <class Pheet, typename TT, class StrategyRetriever>
MergeStrategyHeap<Pheet, TT, StrategyRetriever>::~MergeStrategyHeap() {
	for(auto i = item_heaps.begin(); i != item_heaps.end(); ++i) {
		delete i->second;
	}
	for(auto i = heap_heaps.begin(); i != heap_heaps.end(); ++i) {
		if(i->second != &root_heap) {
			delete i->second;
		}
	}
}

template <class Pheet, typename TT, class StrategyRetriever>
template <class Strategy>
void MergeStrategyHeap<Pheet, TT, StrategyRetriever>::push(T const& item) {
	HeapBase*& bheap = item_heaps[std::type_index(typeid(Strategy))];
	if(bheap == nullptr) {
		bheap = new MergeStrategyItemHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>(sr, heap_heaps);
	}
	MergeStrategyItemHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>* heap = static_cast<MergeStrategyItemHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>*>(bheap);
//	total_size -= heap->perform_cleanup();
	heap->push(item);
	++total_size;
}

template <class Pheet, typename TT, class StrategyRetriever>
TT MergeStrategyHeap<Pheet, TT, StrategyRetriever>::pop() {
//	total_size -= heap->perform_cleanup();
	--total_size;
	return root_heap.pop(/*total_size*/);
}

template <class Pheet, typename TT, class StrategyRetriever>
TT& MergeStrategyHeap<Pheet, TT, StrategyRetriever>::peek() {
	return root_heap.peek();
}

template <class Pheet, typename TT, class StrategyRetriever>
bool MergeStrategyHeap<Pheet, TT, StrategyRetriever>::is_empty() const {
	pheet_assert((total_size == 0) == root_heap.is_empty());
	return root_heap.is_empty();
}

template <class Pheet, typename TT, class StrategyRetriever>
void MergeStrategyHeap<Pheet, TT, StrategyRetriever>::print_name() {
	std::cout << "MergeStrategyHeap";
}

}

#endif /* MERGESTRATEGYHEAP_H_ */
