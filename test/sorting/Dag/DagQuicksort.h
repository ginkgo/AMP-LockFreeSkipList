/*
 * DagQuicksortImpl.h
 *
 *  Created on: 08.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef DAGQUICKSORT_H_
#define DAGQUICKSORT_H_

#include <pheet/pheet.h>
#include <pheet/misc/types.h>
#include <pheet/misc/atomics.h>

#include <functional>
#include <algorithm>

namespace pheet {

template <class Pheet, size_t CUTOFF_LENGTH>
class DagQuicksortImpl : public Pheet::Task {
public:
	typedef DagQuicksortImpl<Pheet, CUTOFF_LENGTH> Self;

	template<size_t NEW_VAL>
		using WITH_CUTOFF_LENGTH = DagQuicksortImpl<Pheet, NEW_VAL>;

	DagQuicksortImpl(unsigned int* data, size_t length);
	virtual ~DagQuicksortImpl();

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
//procs_t const DagQuicksortImpl<Pheet>::max_cpus = Pheet::max_cpus;

template <class Pheet, size_t CUTOFF_LENGTH>
char const DagQuicksortImpl<Pheet, CUTOFF_LENGTH>::name[] = "Dag Quicksort";

//template <class Pheet>
//char const * const DagQuicksortImpl<Pheet>::scheduler_name = Pheet::name;

template <class Pheet, size_t CUTOFF_LENGTH>
DagQuicksortImpl<Pheet, CUTOFF_LENGTH>::DagQuicksortImpl(unsigned int *data, size_t length)
: data(data), length(length) {

}

template <class Pheet, size_t CUTOFF_LENGTH>
DagQuicksortImpl<Pheet, CUTOFF_LENGTH>::~DagQuicksortImpl() {

}

template <class Pheet, size_t CUTOFF_LENGTH>
void DagQuicksortImpl<Pheet, CUTOFF_LENGTH>::operator()() {
	if(length <= 1)
		return;

	unsigned int * middle = std::partition(data, data + length - 1,
		std::bind2nd(std::less<unsigned int>(), *(data + length - 1)));
	size_t pivot = middle - data;
	std::swap(*(data + length - 1), *middle);    // move pivot to middle

	if(pivot >= CUTOFF_LENGTH) {
		Pheet::template
			spawn<Self>(data, pivot);
	}
	else {
		Pheet::template
			call<Self>(data, pivot);
	}
	Pheet::template
		call<Self>(data + pivot + 1, length - pivot - 1);
}

/*
template <class Pheet>
void DagQuicksortImpl<Pheet>::print_results() {
	scheduler.print_performance_counter_values();
}

template <class Pheet>
void DagQuicksortImpl<Pheet>::print_headers() {
	scheduler.print_performance_counter_headers();
}

template <class Pheet>
void DagQuicksortImpl<Pheet>::print_scheduler_name() {
	Pheet::print_name();
}
*/

template<class Pheet>
using DagQuicksort = DagQuicksortImpl<Pheet, 512>;

template<class Pheet>
using DagQuicksortNoCut = DagQuicksortImpl<Pheet, 0>;

}

#endif /* DAGQUICKSORT_H_ */
