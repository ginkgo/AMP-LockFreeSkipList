/*
 * ReferenceSTLSort.h
 *
 *  Created on: 07.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef REFERENCESTLSORT_H_
#define REFERENCESTLSORT_H_

#include <algorithm>
#include <pheet/misc/types.h>

namespace pheet {

template <class Pheet>
class ReferenceSTLSort {
public:
	ReferenceSTLSort(unsigned int* data, size_t length);
	~ReferenceSTLSort();

	void operator()();

	static const char name[];

private:
	unsigned int* start;
	unsigned int* end;
};

template <class Pheet>
const char ReferenceSTLSort<Pheet>::name[] = "ReferenceSTLSort";

template <class Pheet>
ReferenceSTLSort<Pheet>::ReferenceSTLSort(unsigned int* data, size_t length)
: start(data), end(data + length) {

}

template <class Pheet>
ReferenceSTLSort<Pheet>::~ReferenceSTLSort() {

}

template <class Pheet>
void ReferenceSTLSort<Pheet>::operator()() {
	std::sort(start, end);
}

}

#endif /* REFERENCESTLSORT_H_ */
