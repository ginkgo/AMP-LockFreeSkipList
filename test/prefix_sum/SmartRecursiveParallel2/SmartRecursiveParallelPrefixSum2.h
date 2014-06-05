/*
 * SmartRecursiveParallelPrefixSum2.h
 *
 *  Created on: Mar 07, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SMARTRECURSIVEPARALLELPREFIXSUM2_H_
#define SMARTRECURSIVEPARALLELPREFIXSUM2_H_

#include <pheet/pheet.h>
#include <pheet/misc/align.h>
#include "SmartRecursiveParallelPrefixSum2OffsetTask.h"
#include "../RecursiveParallel2/RecursiveParallelPrefixSum2LocalSumTask.h"
#include "SmartRecursiveParallelPrefixSum2PerformanceCounters.h"

#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive>
class SmartRecursiveParallelPrefixSumImpl2 : public Pheet::Task {
public:
	typedef SmartRecursiveParallelPrefixSumImpl2<Pheet, BlockSize, Inclusive> Self;
	typedef SmartRecursiveParallelPrefixSumImpl2<Pheet, BlockSize, false> ExclusiveSelf;
	typedef SmartRecursiveParallelPrefixSum2OffsetTask<Pheet, BlockSize, Inclusive> OffsetTask;
	typedef RecursiveParallelPrefixSum2LocalSumTask<Pheet, BlockSize, Inclusive> LocalSumTask;
	typedef SmartRecursiveParallelPrefixSum2PerformanceCounters<Pheet> PerformanceCounters;

	SmartRecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length)
	:data(data), length(length), step(1), root(true) {

	}

	SmartRecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, size_t step, bool root)
	:data(data), length(length), step(step), root(root) {

	}

	SmartRecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, PerformanceCounters& pc)
	:data(data), length(length), step(1), root(true), pc(pc) {

	}

	SmartRecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, size_t step, bool root, PerformanceCounters& pc)
	:data(data), length(length), step(step), root(root), pc(pc) {

	}

	virtual ~SmartRecursiveParallelPrefixSumImpl2() {}

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

			std::atomic<size_t> sequential(0);

			// Calculate offsets
			Pheet::template finish<OffsetTask>(data, auxiliary_data.ptr(), num_blocks, length, 0, sequential);
			size_t seq = sequential.load(std::memory_order_relaxed);
			pc.blocks.add(num_blocks);
			pc.preprocessed_blocks.add(seq);
			if(seq < num_blocks) {
				// Prefix sum on offsets
				pheet_assert(seq > 0);
				Pheet::template finish<ExclusiveSelf>(auxiliary_data.ptr() + seq - 1, num_blocks + 1 - seq, pc);

				// Calculate local prefix sums based on offset
				Pheet::template finish<LocalSumTask>(data + (seq * BlockSize), auxiliary_data.ptr() + seq, num_blocks - seq, length - (seq * BlockSize));
			}
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
char const SmartRecursiveParallelPrefixSumImpl2<Pheet, BlockSize, Inclusive>::name[] = "SmartRecursiveParallelPrefixSum2";

template <class Pheet>
using SmartRecursiveParallelPrefixSum2 = SmartRecursiveParallelPrefixSumImpl2<Pheet, 4096, true>;

} /* namespace pheet */
#endif /* SMARTRECURSIVEPARALLELPREFIXSUM2_H_ */
