/*
 * LocalStrategyTaskStorage.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LOCALSTRATEGYTASKSTORAGE_H_
#define LOCALSTRATEGYTASKSTORAGE_H_

#include <iostream>

#include "../../StrategyHeap/Basic/BasicStrategyHeap.h"

namespace pheet {

template <class Pheet, typename TT>
struct LocalStrategyTaskStorageItem {
	typename Pheet::Scheduler::BaseStrategy* strategy;
	TT item;
};

template <class Pheet, class Item>
struct LocalStrategyTaskStorageStrategyRetriever {
	typename Pheet::Scheduler::BaseStrategy& operator()(Item& item) {
		return *(item.strategy);
	}

	inline bool is_active(Item& item) { return true; }
	inline void mark_removed(Item& item) {}
};

/*
 * Provides a task storage for a single thread. no thread-safety and stealing is not allowed
 */
template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
class LocalStrategyTaskStorageImpl {
public:
	typedef TT T;
	typedef LocalStrategyTaskStorageItem<Pheet, T> Item;
	typedef LocalStrategyTaskStorageStrategyRetriever<Pheet, Item> StrategyRetriever;
	typedef StrategyHeapT<Pheet, Item, StrategyRetriever> StrategyHeap;
	typedef typename StrategyHeap::PerformanceCounters PerformanceCounters;

	LocalStrategyTaskStorageImpl(PerformanceCounters& pc);
	~LocalStrategyTaskStorageImpl();

	template <class Strategy>
	void push(Strategy&& s, T item);
	T pop();
	T& peek();

	inline size_t size() const { return get_length(); }
	size_t get_length() const;
	inline bool empty() const { return is_empty(); }
	bool is_empty() const;
	inline bool is_full() const { return false; }

	static void print_name();
private:
	StrategyHeap heap;
	size_t sz;
};

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
LocalStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT>::LocalStrategyTaskStorageImpl(PerformanceCounters& pc)
:heap(StrategyRetriever(), pc), sz(0) {

}

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
LocalStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT>::~LocalStrategyTaskStorageImpl() {

}

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
template <class Strategy>
void LocalStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT>::push(Strategy&& s, T item) {
	Item it;
	it.strategy = new Strategy(std::move(s));
	it.item = item;
	heap.push<Strategy>(it);
	++sz;
}

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
TT LocalStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT>::pop() {
	if(empty()) {
		return nullable_traits<T>::null_value;
	}
	Item it = heap.pop();
	--sz;
	delete it.strategy;
	return it.item;
}

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
TT& LocalStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT>::peek() {
	return heap.peek().item;
}

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
bool LocalStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT>::is_empty() const {
	return sz == 0;
}

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
size_t LocalStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT>::get_length() const {
	return sz;
}

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT>
void LocalStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT>::print_name() {
	std::cout << "LocalStrategyTaskStorage<";
	StrategyHeap::print_name();
	std::cout << ">";
}

template <class Pheet, typename T>
using LocalStrategyTaskStorage = LocalStrategyTaskStorageImpl<Pheet, T, BasicStrategyHeap>;

}

#endif /* LOCALSTRATEGYTASKSTORAGE_H_ */
