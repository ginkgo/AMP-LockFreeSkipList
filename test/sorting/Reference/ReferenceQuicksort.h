/*
 * ReferenceQuicksort.h
 *
 *  Created on: 21.04.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef REFERENCEQUICKSORT_H_
#define REFERENCEQUICKSORT_H_

#include <functional>
#include <algorithm>

/*
 *
 */
namespace pheet {

template <class Pheet>
class ReferenceQuicksort {
public:
	ReferenceQuicksort(unsigned int* data, size_t length);
	~ReferenceQuicksort();

	void operator()();

	static const char name[];
private:
	void sort(unsigned int* data, size_t length);

	unsigned int* data;
	size_t length;
};

template <class Pheet>
const char ReferenceQuicksort<Pheet>::name[] = "ReferenceQuicksort";

template <class Pheet>
ReferenceQuicksort<Pheet>::ReferenceQuicksort(unsigned int* data, size_t length)
: data(data), length(length) {

}

template <class Pheet>
ReferenceQuicksort<Pheet>::~ReferenceQuicksort() {

}

template <class Pheet>
void ReferenceQuicksort<Pheet>::operator()() {
	sort(data, length);
}

template <class Pheet>
void ReferenceQuicksort<Pheet>::sort(unsigned int* data, size_t length) {
	if(length <= 1)
		return;

	unsigned int * middle = std::partition(data, data + length - 1,
		std::bind2nd(std::less<unsigned int>(), *(data + length - 1)));
	size_t pivot = middle - data;
	std::swap(*(data + length - 1), *middle);    // move pivot to middle

	sort(data, pivot);
	sort(data + pivot + 1, length - pivot - 1);
}

}

#endif /* REFERENCEQUICKSORT_H_ */
