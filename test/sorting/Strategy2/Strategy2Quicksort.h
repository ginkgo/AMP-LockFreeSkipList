/*
 * Strategy2Quicksort.h
 *
 *  Created on: 03.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGY2QUICKSORT_H_
#define STRATEGY2QUICKSORT_H_

#include <pheet/pheet.h>
#include <pheet/misc/types.h>
#include <pheet/misc/atomics.h>

#include <functional>
#include <algorithm>

#include "Strategy2QuicksortStrategy.h"

namespace pheet {

template <class Pheet, size_t CUTOFF_LENGTH>
class Strategy2QuicksortImpl : public Pheet::Task {
public:
	typedef Strategy2QuicksortImpl<Pheet, CUTOFF_LENGTH> Self;

	template<size_t NEW_VAL>
		using WITH_CUTOFF_LENGTH = Strategy2QuicksortImpl<Pheet, NEW_VAL>;

	Strategy2QuicksortImpl(unsigned int* data, size_t length);
	virtual ~Strategy2QuicksortImpl();

	virtual void operator()();
/*	void print_results();

	void print_headers();

	static void print_scheduler_name();
*/
//	static procs_t const max_cpus;
	static char const name[];
//	static char const * const scheduler_name;

private:
	unsigned int* data;
	size_t length;
};

//template <class Pheet>
//procs_t const Strategy2QuicksortImpl<Pheet>::max_cpus = Pheet::max_cpus;

template <class Pheet, size_t CUTOFF_LENGTH>
char const Strategy2QuicksortImpl<Pheet, CUTOFF_LENGTH>::name[] = "Strategy Quicksort";

//template <class Pheet>
//char const * const Strategy2QuicksortImpl<Pheet>::scheduler_name = Pheet::name;

template <class Pheet, size_t CUTOFF_LENGTH>
Strategy2QuicksortImpl<Pheet, CUTOFF_LENGTH>::Strategy2QuicksortImpl(unsigned int *data, size_t length)
: data(data), length(length) {

}

template <class Pheet, size_t CUTOFF_LENGTH>
Strategy2QuicksortImpl<Pheet, CUTOFF_LENGTH>::~Strategy2QuicksortImpl() {

}

template <class Pheet, size_t CUTOFF_LENGTH>
void Strategy2QuicksortImpl<Pheet, CUTOFF_LENGTH>::operator()() {
	if(length <= 1)
		return;

	unsigned int * middle = std::partition(data, data + length - 1,
		std::bind2nd(std::less<unsigned int>(), *(data + length - 1)));
	size_t pivot = middle - data;
	std::swap(*(data + length - 1), *middle);    // move pivot to middle

	if(pivot > CUTOFF_LENGTH) {
		Pheet::template
			spawn_s<Self>(Strategy2QuicksortStrategy<Pheet>(pivot),
				data, pivot);
	}
	else {
		Pheet::template
			call<Self>(data, pivot);
	}
	Pheet::template
		call<Self>(data + pivot + 1, length - pivot - 1);
/*
	Pheet::template
		spawn_s<Self>(Strategy2QuicksortStrategy<Pheet>(length - pivot),
				data + pivot + 1, length - pivot - 1);*/
}

template<class Pheet>
using Strategy2Quicksort = Strategy2QuicksortImpl<Pheet, 512>;

}

#endif /* STRATEGY2QUICKSORT_H_ */
