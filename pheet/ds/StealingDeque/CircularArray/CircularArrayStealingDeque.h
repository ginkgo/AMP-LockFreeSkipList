/*
 * CircularArrayStealingDeque.h
 *
 *  Created on: 12.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef CIRCULARARRAYSTEALINGDEQUE_H_
#define CIRCULARARRAYSTEALINGDEQUE_H_

#include "../../../settings.h"
#include "../../../misc/type_traits.h"
#include "../../../misc/atomics.h"
#include "CircularArrayStealingDequePerformanceCounters.h"

#include <limits>
#include <iostream>

namespace pheet {

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
class CircularArrayStealingDequeImpl {
public:
	typedef TT T;
	typedef CircularArrayStealingDequePerformanceCounters<Pheet> PerformanceCounters;

	template<template <class P, typename S> class NewCA>
		using WithCircularArray = CircularArrayStealingDequeImpl<Pheet, TT, NewCA>;

	template <class P, class TTT>
	using BT = CircularArrayStealingDequeImpl<P, TTT, CircularArray>;

	CircularArrayStealingDequeImpl();
	CircularArrayStealingDequeImpl(PerformanceCounters& pc);
	CircularArrayStealingDequeImpl(size_t initial_capacity);
	CircularArrayStealingDequeImpl(size_t initial_capacity, PerformanceCounters& pc);
	~CircularArrayStealingDequeImpl();

	void push(T item);
	T pop();
	T peek();
	T steal();
	T steal(PerformanceCounters& pc);

	T steal_push(CircularArrayStealingDequeImpl<Pheet, TT, CircularArray> &other);

	size_t get_length() const;
	bool is_empty() const;
	bool is_full() const;

private:
	size_t top;
	size_t bottom;

	static const size_t top_mask;
	static const size_t top_stamp_mask;
	static const size_t top_stamp_add;

	static const T null_element;

	CircularArray<Pheet, T> data;

	PerformanceCounters pc;
};

// Upper 4th of size_t is reserved for stamp. The rest is for the actual content
template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
const size_t CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::top_mask =
		(std::numeric_limits<size_t>::max() >> (std::numeric_limits<size_t>::digits >> 2));

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
const size_t CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::top_stamp_mask =
		(std::numeric_limits<size_t>::max() ^ CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::top_mask);

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
const size_t CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::top_stamp_add =
		(((size_t)1) << (std::numeric_limits<size_t>::digits - (std::numeric_limits<size_t>::digits >> 2)));

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
const TT CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::null_element = nullable_traits<T>::null_value;

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::CircularArrayStealingDequeImpl()
: top(0), bottom(0), data() {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::CircularArrayStealingDequeImpl(PerformanceCounters& pc)
: top(0), bottom(0), data(), pc(pc) {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::CircularArrayStealingDequeImpl(size_t initial_capacity)
: top(0), bottom(0), data(initial_capacity) {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::CircularArrayStealingDequeImpl(size_t initial_capacity, PerformanceCounters& pc)
: top(0), bottom(0), data(initial_capacity), pc(pc) {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::~CircularArrayStealingDequeImpl() {

}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
void CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::push(T item) {
	pheet_assert(bottom >= (top & top_mask));
	if((bottom - (top & top_mask)) >= (data.get_capacity()))
	{
		data.grow(bottom, top & top_mask);
		MEMORY_FENCE ();

		// after resizing we have to make sure no thread can succeed that read from the old indices
		// We do this by incrementing the stamp of top
		size_t old_top = top;
		size_t new_top = old_top + top_stamp_add;

		// We don't care if we succeed as long as some thread succeeded to change the stamp
		SIZET_CAS(&top, old_top, new_top);
	}

	data.put(bottom, item);

	// Make sure no thread sees new bottom before data has been put
	MEMORY_FENCE();
	bottom++;
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
TT CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::pop() {
	if(bottom == (top & top_mask))
		return null_element;

	bottom--;

	// Make sure the write to bottom is seen by all threads
	// requires a write barrier
	MEMORY_FENCE ();

	T ret = data.get(bottom);

	size_t old_top = top;
	size_t masked_top = old_top & top_mask;
	if(bottom > masked_top)
	{
		return ret;
	}
	if(bottom == masked_top)
	{
		// Increment stamp (should wrap around)
		size_t new_top = old_top + top_stamp_add;

		pc.num_pop_cas.incr();
		if(SIZET_CAS(&top, old_top, new_top))
		{
		//	std::cout << "pop " << bottom << std::endl;
			return ret;
		}
		else {
			// Some other process was faster - the queue is now empty
			bottom++;
		}
	}
	else {
		pheet_assert(bottom == masked_top - 1);
		bottom = masked_top;
	}
	return null_element;
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
TT CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::peek() {
	if(bottom == (top & top_mask))
		return null_element;

	return data.get(bottom);
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
inline TT CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::steal() {
	PerformanceCounters pc;
	return steal(pc);
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
TT CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::steal(PerformanceCounters& pc) {
	size_t old_top = top;
	MEMORY_FENCE();

	size_t masked_top = old_top & top_mask;
	if(bottom <= masked_top) {
		return null_element;
	}

	T ret = data.get(masked_top);

	size_t new_top = old_top + 1 + top_stamp_add;
	if(UINT_CAS(&top, old_top, new_top))
	{
		pc.num_stolen.incr();
		return ret;
	}

	// if we encounter a race, just return NULL
	// This might even happen on well-filled queues
	return null_element;
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
TT CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::steal_push(CircularArrayStealingDequeImpl<Pheet, TT, CircularArray> &other) {
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
size_t CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::get_length() const {
	return (bottom - (top & top_mask));
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
bool CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::is_empty() const {
	return get_length() == 0;
}

template <class Pheet, typename TT, template <class P, typename S> class CircularArray>
bool CircularArrayStealingDequeImpl<Pheet, TT, CircularArray>::is_full() const {
	return (!data.is_growable()) && (get_length() >= data.get_capacity());
}

template<class Pheet, typename T>
using CircularArrayStealingDequeDefaultCircularArray = typename Pheet::CDS::template CircularArray<T>;

template<class Pheet, typename T>
using CircularArrayStealingDeque = CircularArrayStealingDequeImpl<Pheet, T, CircularArrayStealingDequeDefaultCircularArray>;


}

#endif /* CIRCULARARRAYSTEALINGDEQUE_H_ */
