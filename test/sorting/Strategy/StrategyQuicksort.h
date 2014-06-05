/*
 * StrategyQuicksort.h
 *
 *  Created on: 03.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYQUICKSORT_H_
#define STRATEGYQUICKSORT_H_

#include <pheet/pheet.h>
#include <pheet/misc/types.h>
#include <pheet/misc/atomics.h>

#include <functional>
#include <algorithm>

#include "StrategyQuicksortStrategy.h"

namespace pheet {

template <class Pheet, size_t CUTOFF_LENGTH>
class StrategyQuicksortImpl : public Pheet::Task {
public:
	typedef StrategyQuicksortImpl<Pheet, CUTOFF_LENGTH> Self;

	template<size_t NEW_VAL>
		using WITH_CUTOFF_LENGTH = StrategyQuicksortImpl<Pheet, NEW_VAL>;

	StrategyQuicksortImpl(unsigned int* data, size_t length);
	virtual ~StrategyQuicksortImpl();

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
//procs_t const StrategyQuicksortImpl<Pheet>::max_cpus = Pheet::max_cpus;

template <class Pheet, size_t CUTOFF_LENGTH>
char const StrategyQuicksortImpl<Pheet, CUTOFF_LENGTH>::name[] = "Strategy Quicksort";

//template <class Pheet>
//char const * const StrategyQuicksortImpl<Pheet>::scheduler_name = Pheet::name;

template <class Pheet, size_t CUTOFF_LENGTH>
StrategyQuicksortImpl<Pheet, CUTOFF_LENGTH>::StrategyQuicksortImpl(unsigned int *data, size_t length)
: data(data), length(length) {

}

template <class Pheet, size_t CUTOFF_LENGTH>
StrategyQuicksortImpl<Pheet, CUTOFF_LENGTH>::~StrategyQuicksortImpl() {

}

template <class Pheet, size_t CUTOFF_LENGTH>
void StrategyQuicksortImpl<Pheet, CUTOFF_LENGTH>::operator()() {
	if(length <= 1)
		return;

	unsigned int * middle = std::partition(data, data + length - 1,
		std::bind2nd(std::less<unsigned int>(), *(data + length - 1)));
	size_t pivot = middle - data;
	std::swap(*(data + length - 1), *middle);    // move pivot to middle

	if(pivot > CUTOFF_LENGTH) {
		Pheet::template
			spawn_s<Self>(StrategyQuicksortStrategy<Pheet>(pivot),
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
		spawn_s<Self>(StrategyQuicksortStrategy<Pheet>(length - pivot),
				data + pivot + 1, length - pivot - 1);*/
}

template<class Pheet>
using StrategyQuicksort = StrategyQuicksortImpl<Pheet, 512>;

}

#endif /* STRATEGYQUICKSORT_H_ */
