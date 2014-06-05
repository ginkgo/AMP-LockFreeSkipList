/*
 * CircularArrayStealingDeque.h
 *
 *  Created on: 12.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef CIRCULARARRAYSTEALINGDEQUE11_H_
#define CIRCULARARRAYSTEALINGDEQUE11_H_

#include "../../../settings.h"
#include "../../../misc/type_traits.h"
#include "CircularArrayStealingDeque11PerformanceCounters.h"

#include <limits>
#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
class CircularArrayStealingDeque11Impl {
public:
	typedef TT T;
	typedef CircularArrayStealingDeque11PerformanceCounters<Pheet> PerformanceCounters;

	template<template <class P, typename S> class NewCA>
		using WithCircularArray = CircularArrayStealingDeque11Impl<Pheet, TT, NewCA>;

	template <class P, class TTT>
	using BT = CircularArrayStealingDeque11Impl<P, TTT, CircularArray>;

	CircularArrayStealingDeque11Impl();
	CircularArrayStealingDeque11Impl(PerformanceCounters& pc);
	CircularArrayStealingDeque11Impl(size_t initial_capacity);
	CircularArrayStealingDeque11Impl(size_t initial_capacity, PerformanceCounters& pc);
	~CircularArrayStealingDeque11Impl();

	void push(T item);
	T pop();
	T peek();
	T steal();
	T steal(PerformanceCounters& pc);

	T steal_push(CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray> &other);

	size_t get_length() const;
	bool is_empty() const;
	bool is_full() const;

private:
	std::atomic<size_t> top;
	std::atomic<size_t> bottom;

	static const size_t top_mask;
	static const size_t top_stamp_mask;
	static const size_t top_stamp_add;

	static const T null_element;

	CircularArray<Pheet, T> data;

	PerformanceCounters pc;
};

// Upper 4th of size_t is reserved for stamp. The rest is for the actual content
template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
const size_t CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::top_mask =
		(std::numeric_limits<size_t>::max() >> (std::numeric_limits<size_t>::digits >> 2));

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
const size_t CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::top_stamp_mask =
		(std::numeric_limits<size_t>::max() ^ CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::top_mask);

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
const size_t CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::top_stamp_add =
		(((size_t)1) << (std::numeric_limits<size_t>::digits - (std::numeric_limits<size_t>::digits >> 2)));

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
const TT CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::null_element = nullable_traits<T>::null_value;

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::CircularArrayStealingDeque11Impl()
: top(0, memory_order_release), bottom(0, memory_order_release), data() {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::CircularArrayStealingDeque11Impl(PerformanceCounters& pc)
: top(0, memory_order_release), bottom(0, memory_order_release), data(), pc(pc) {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::CircularArrayStealingDeque11Impl(size_t initial_capacity)
: top(0, memory_order_release), bottom(0, memory_order_release), data(initial_capacity) {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::CircularArrayStealingDeque11Impl(size_t initial_capacity, PerformanceCounters& pc)
: top(0, memory_order_release), bottom(0, memory_order_release), data(initial_capacity), pc(pc) {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::~CircularArrayStealingDeque11Impl() {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
void CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::push(T item) {
	size_t b = bottom.load(std::memory_order_relaxed);
	size_t t = top.load(std::memory_order_relaxed);
	pheet_assert(b >= (t & top_mask));
	if((b - (t & top_mask)) >= (data.get_capacity()))
	{
		data.grow(b, t & top_mask);
		MEMORY_FENCE ();

		// after resizing we have to make sure no thread can succeed that read from the old indices
		// We do this by incrementing the stamp of top
		size_t old_top = t;
		size_t new_top = old_top + top_stamp_add;

		// We don't care if we succeed as long as some thread succeeded to change the stamp
		top.compare_exchange_strong(old_top, new_top, std::memory_order_release, std::memory_order_relaxed);
	}

	data.put(b, item);

	// Make sure no thread sees new bottom before data has been put
	bottom.store(b + 1, std::memory_order_release);
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
TT CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::pop() {
	size_t b = bottom.load(std::memory_order_relaxed);
	size_t t = top.load(std::memory_order_relaxed);
	if(b == (t & top_mask))
		return null_element;

	--b;
	bottom.store(b, std::memory_order_seq_cst);
//	bottom--;

	// Make sure the write to bottom is seen by all threads
	// requires a write barrier
//	MEMORY_FENCE ();

	T ret = data.get(b);

	t = top.load(std::memory_order_relaxed);

	size_t old_top = t;
	size_t masked_top = old_top & top_mask;
	if(b > masked_top)
	{
		return ret;
	}
	if(b == masked_top)
	{
		// Increment stamp (should wrap around)
		size_t new_top = old_top + top_stamp_add;

		pc.num_pop_cas.incr();
		if(top.compare_exchange_strong(old_top, new_top, std::memory_order_release, std::memory_order_relaxed))
		{
		//	std::cout << "pop " << bottom << std::endl;
			return ret;
		}
		else {
			// Some other process was faster - the queue is now empty
			bottom.store(b + 1, std::memory_order_release);
		}
	}
	else {
		pheet_assert(b == masked_top - 1);
		bottom.store(masked_top, std::memory_order_release);
	}
	return null_element;
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
TT CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::peek() {
	size_t b = bottom.load(std::memory_order_relaxed);
	size_t t = top.load(std::memory_order_relaxed);
	if(b == (t & top_mask))
		return null_element;

	return data.get(b);
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
inline TT CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::steal() {
	PerformanceCounters pc;
	return steal(pc);
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
TT CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::steal(PerformanceCounters& pc) {
	size_t old_top = top.load(std::memory_order_acquire);
	size_t b = bottom.load(std::memory_order_relaxed);

	size_t masked_top = old_top & top_mask;
	if(b <= masked_top) {
		return null_element;
	}

	T ret = data.get(masked_top);

	size_t new_top = old_top + 1 + top_stamp_add;
	if(top.compare_exchange_strong(old_top, new_top, std::memory_order_seq_cst, std::memory_order_relaxed))
	{
		pc.num_stolen.incr();
		return ret;
	}

	// if we encounter a race, just return NULL
	// This might even happen on well-filled queues
	return null_element;
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
TT CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::steal_push(CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray> &other) {
	T prev = null_element;
	T curr = null_element;
	size_t max_steal = get_length() / 2;

	for(size_t i = 0; i < max_steal; i++) {
		curr = steal();
		if(curr == null_element) {
			return prev;
		}
		else if(prev != null_element) {
			other.pc.num_stolen.incr();
			other.push(prev);
		}
		else {
			other.pc.num_stolen.incr();
		}
		prev = curr;
	}
	return prev;
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
size_t CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::get_length() const {
	return (bottom - (top & top_mask));
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
bool CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::is_empty() const {
	return get_length() == 0;
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
bool CircularArrayStealingDeque11Impl<Pheet, TT, CircularArray>::is_full() const {
	return (!data.is_growable()) && (get_length() >= data.get_capacity());
}

template<class Pheet, typename T>
using CircularArrayStealingDeque11DefaultCircularArray = typename Pheet::CDS::template CircularArray<T>;

template<class Pheet, typename T>
using CircularArrayStealingDeque11 = CircularArrayStealingDeque11Impl<Pheet, T, CircularArrayStealingDeque11DefaultCircularArray>;


}

#endif /* CIRCULARARRAYSTEALINGDEQUE11_H_ */
