/*
 * ReferenceHeapSortImpl.h
 *
 *  Created on: Nov 30, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef REFERENCEHEAPSORT_H_
#define REFERENCEHEAPSORT_H_

#include <pheet/pheet.h>

namespace pheet {

template <class Pheet, template <class, typename T, typename Comp> class PriorityQueue>
class ReferenceHeapSortImpl {
public:
	template<template <class, typename T, typename Comp> class NewPQ>
	using WithPriorityQueue = ReferenceHeapSortImpl<Pheet, NewPQ>;

	template <class NP>
	using BT = ReferenceHeapSortImpl<NP, PriorityQueue>;

	ReferenceHeapSortImpl(unsigned int* data, size_t length);
	~ReferenceHeapSortImpl();

	void operator()();

	static const char name[];

private:
	void sort(unsigned int* data, size_t length);

	unsigned int* data;
	size_t length;
};

template <class Pheet, template <class, typename T, typename Comp> class PriorityQueue>
const char ReferenceHeapSortImpl<Pheet, PriorityQueue>::name[] = "ReferenceHeapSortImpl";

template <class Pheet, template <class, typename T, typename Comp> class PriorityQueue>
ReferenceHeapSortImpl<Pheet, PriorityQueue>::ReferenceHeapSortImpl(unsigned int* data, size_t length)
: data(data), length(length) {

}

template <class Pheet, template <class, typename T, typename Comp> class PriorityQueue>
ReferenceHeapSortImpl<Pheet, PriorityQueue>::~ReferenceHeapSortImpl() {

}

template <class Pheet, template <class, typename T, typename Comp> class PriorityQueue>
void ReferenceHeapSortImpl<Pheet, PriorityQueue>::operator()() {
	sort(data, length);
}

template <class Pheet, template <class, typename T, typename Comp> class PriorityQueue>
void ReferenceHeapSortImpl<Pheet, PriorityQueue>::sort(unsigned int* data, size_t length) {
	if(length <= 1)
		return;

	PriorityQueue<Pheet, unsigned int, std::less<unsigned int> > pq;

	for(size_t i = 0; i < length; ++i) {
		pq.push(data[i]);
	}

	while(!pq.is_empty()) {
		unsigned int tmp = pq.pop();
		data[pq.get_length()] = tmp;
	}
}

template <class Pheet, typename T, typename Comp>
using ReferenceHeapSortDefaultPQ = typename Pheet::DataStructures::template PriorityQueue<T, Comp>;

template<class Pheet>
using ReferenceHeapSort = ReferenceHeapSortImpl<Pheet, ReferenceHeapSortDefaultPQ>;

}

#endif /* REFERENCEHEAPSORT_H_ */
