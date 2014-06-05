/*
 * StrategyRecursiveParallelPrefixSum2.h
 *
 *  Created on: Mar 07, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGYRECURSIVEPARALLELPREFIXSUM2_H_
#define STRATEGYRECURSIVEPARALLELPREFIXSUM2_H_

#include <pheet/pheet.h>
#include <pheet/misc/align.h>
#include "StrategyRecursiveParallelPrefixSum2OffsetTask.h"
#include "../RecursiveParallel2/RecursiveParallelPrefixSum2LocalSumTask.h"
#include "StrategyRecursiveParallelPrefixSum2PerformanceCounters.h"

#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive>
class StrategyRecursiveParallelPrefixSumImpl2 : public Pheet::Task {
public:
	typedef StrategyRecursiveParallelPrefixSumImpl2<Pheet, BlockSize, Inclusive> Self;
	typedef StrategyRecursiveParallelPrefixSumImpl2<Pheet, BlockSize, false> ExclusiveSelf;
	typedef StrategyRecursiveParallelPrefixSum2OffsetTask<Pheet, BlockSize, Inclusive> OffsetTask;
	typedef RecursiveParallelPrefixSum2LocalSumTask<Pheet, BlockSize, Inclusive> LocalSumTask;
	typedef StrategyRecursiveParallelPrefixSum2PerformanceCounters<Pheet> PerformanceCounters;

	StrategyRecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length)
	:data(data), length(length), step(1), root(true) {

	}

	StrategyRecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, size_t step, bool root)
	:data(data), length(length), step(step), root(root) {

	}

	StrategyRecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, PerformanceCounters& pc)
	:data(data), length(length), step(1), root(true), pc(pc) {

	}

	StrategyRecursiveParallelPrefixSumImpl2(unsigned int* data, size_t length, size_t step, bool root, PerformanceCounters& pc)
	:data(data), length(length), step(step), root(root), pc(pc) {

	}

	virtual ~StrategyRecursiveParallelPrefixSumImpl2() {}

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
			Pheet::template finish<OffsetTask>(data, auxiliary_data.ptr(), num_blocks, length, 0, sequential, Pheet::get_place());
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
char const StrategyRecursiveParallelPrefixSumImpl2<Pheet, BlockSize, Inclusive>::name[] = "StrategyRecursiveParallelPrefixSum2";

template <class Pheet>
using StrategyRecursiveParallelPrefixSum2 = StrategyRecursiveParallelPrefixSumImpl2<Pheet, 4096, true>;

} /* namespace pheet */
#endif /* STRATEGYRECURSIVEPARALLELPREFIXSUM2_H_ */
