/*
 * MixedModeQuicksort.h
 *
 *  Created on: 30.07.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef MIXEDMODEQUICKSORT_H_
#define MIXEDMODEQUICKSORT_H_

#include <pheet/pheet.h>
#include <pheet/misc/types.h>
#include <pheet/misc/atomics.h>

#include "MixedModeQuicksortTask.h"

namespace pheet {

template <class Pheet, size_t BLOCK_SIZE = 4096>
class MixedModeQuicksortImpl : public Pheet::Task {
public:
	MixedModeQuicksortImpl(unsigned int* data, size_t length);
	~MixedModeQuicksortImpl();

	void operator()();

	static char const name[];

private:
	unsigned int* data;
	size_t length;
};

template <class Pheet, size_t BLOCK_SIZE>
char const MixedModeQuicksortImpl<Pheet, BLOCK_SIZE>::name[] = "MixedMode Quicksort";

template <class Pheet, size_t BLOCK_SIZE>
MixedModeQuicksortImpl<Pheet, BLOCK_SIZE>::MixedModeQuicksortImpl(unsigned int *data, size_t length)
: data(data), length(length) {

}

template <class Pheet, size_t BLOCK_SIZE>
MixedModeQuicksortImpl<Pheet, BLOCK_SIZE>::~MixedModeQuicksortImpl() {

}

template <class Pheet, size_t BLOCK_SIZE>
void MixedModeQuicksortImpl<Pheet, BLOCK_SIZE>::operator()() {
	Pheet::Environment::template spawn_nt<MixedModeQuicksortTask<Pheet, BLOCK_SIZE> >(((length / BLOCK_SIZE) / 8) + 1, data, length);
}
/*
template <class Pheet, size_t BLOCK_SIZE>
void MixedModeQuicksortImpl<Pheet, BLOCK_SIZE>::print_results() {
	scheduler.print_performance_counter_values();
}

template <class Pheet, size_t BLOCK_SIZE>
void MixedModeQuicksortImpl<Pheet, BLOCK_SIZE>::print_headers() {
	scheduler.print_performance_counter_headers();
}

template <class Pheet, size_t BLOCK_SIZE>
void MixedModeQuicksortImpl<Pheet, BLOCK_SIZE>::print_scheduler_name() {
	Pheet::print_name();
}*/

template <class Pheet>
using MixedModeQuicksort = MixedModeQuicksortImpl<Pheet, 4096>;

}

#endif /* MIXEDMODEQUICKSORT_H_ */
