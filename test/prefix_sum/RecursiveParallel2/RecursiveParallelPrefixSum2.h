/*
 * RecursiveParallelPrefixSum2.h
 *
 *  Created on: Mar 07, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUM2_H_
#define RECURSIVEPARALLELPREFIXSUM2_H_

#include <iostream>
#include <pheet/pheet.h>
#include <pheet/misc/align.h>
#include "RecursiveParallelPrefixSum2OffsetTask.h"
#include "RecursiveParallelPrefixSum2LocalSumTask.h"
#include "RecursiveParallelPrefixSum2PerformanceCounters.h"

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive>
class RecursiveParallelPrefixSumImpl2 : public Pheet::Task {
public:
	typedef RecursiveParallelPrefixSumImpl2<Pheet, BlockSize, Inclusive> Self;
	typedef RecursiveParallelPrefixSumImpl2<Pheet, BlockSize, false> ExclusiveSelf;
	typedef RecursiveParallelPrefixSum2OffsetTask<Pheet, BlockSize> OffsetTask;
	typedef RecursiveParallelPrefixSum2LocalSumTask<Pheet, BlockSize, Inclusive> LocalSumTask;
	typedef RecursiveParallelPrefixSum2PerformanceCounters<Pheet> PerformanceCounters;

	RecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length)
	:data(data), length(length), step(1), root(true) {

	}

	RecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, size_t step, bool root)
	:data(data), length(length), step(step), root(root) {

	}

	RecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, PerformanceCounters& pc)
	:data(data), length(length), step(1), root(true), pc(pc) {

	}

	RecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, size_t step, bool root, PerformanceCounters& pc)
	:data(data), length(length), step(step), root(root), pc(pc) {

	}

	virtual ~RecursiveParallelPrefixSumImpl2() {}

	virtual void operator()() {
		if(length <= BlockSize) {
			unsigned int sum = 0;
			for(size_t i = 0; i < length; ++i) {
				unsigned int tmp = data[i];
				if(Inclusive) {
					sum += tmp;
					data[i] = sum;
				}
				else {
					data[i] = sum;
					sum += tmp;
				}
			}
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;

			aligned_data<unsigned int, 64> auxiliary_data(num_blocks);

			// Calculate offsets
			Pheet::template finish<OffsetTask>(data, auxiliary_data.ptr(), num_blocks, length);
			// Prefix sum on offsets
			Pheet::template finish<ExclusiveSelf>(auxiliary_data.ptr(), num_blocks, pc);
			// Calculate local prefix sums based on offset
			Pheet::template finish<LocalSumTask>(data, auxiliary_data.ptr(), num_blocks, length);

		}
	}

	static char const name[];

private:
	unsigned int* data;
	size_t length;
	size_t step;
	bool root;
	PerformanceCounters pc;
};

template <class Pheet, size_t BlockSize, bool Inclusive>
char const RecursiveParallelPrefixSumImpl2<Pheet, BlockSize, Inclusive>::name[] = "RecursiveParallelPrefixSum2";

template <class Pheet>
using RecursiveParallelPrefixSum2 = RecursiveParallelPrefixSumImpl2<Pheet, 4096, true>;

} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUM_H_ */
