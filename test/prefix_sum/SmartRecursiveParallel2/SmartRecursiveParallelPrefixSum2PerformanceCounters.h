/*
 * SmartRecursiveParallelPrefixSum2PerformanceCounters.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SMARTRECURSIVEPARALLELPREFIXSUM2PERFORMANCECOUNTERS_H_
#define SMARTRECURSIVEPARALLELPREFIXSUM2PERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <iostream>

namespace pheet {

template <class Pheet>
class SmartRecursiveParallelPrefixSum2PerformanceCounters {
public:
	typedef SmartRecursiveParallelPrefixSum2PerformanceCounters<Pheet> Self;

	SmartRecursiveParallelPrefixSum2PerformanceCounters() {}
	SmartRecursiveParallelPrefixSum2PerformanceCounters(Self& other)
	:blocks(other.blocks), preprocessed_blocks(other.preprocessed_blocks) {}
	SmartRecursiveParallelPrefixSum2PerformanceCounters(Self&& other)
	:blocks(other.blocks), preprocessed_blocks(other.preprocessed_blocks) {}
	~SmartRecursiveParallelPrefixSum2PerformanceCounters() {}

	static void print_headers() {
		BasicPerformanceCounter<Pheet, prefix_sum_log_pf_blocks>::print_header("blocks\t");
		BasicPerformanceCounter<Pheet, prefix_sum_log_pf_preprocessed_blocks>::print_header("preprocessed_blocks\t");
	}
	void print_values() {
		blocks.print("%lu\t");
		preprocessed_blocks.print("%lu\t");
	}

	BasicPerformanceCounter<Pheet, prefix_sum_log_pf_blocks> blocks;
	BasicPerformanceCounter<Pheet, prefix_sum_log_pf_preprocessed_blocks> preprocessed_blocks;
};

} /* namespace pheet */
#endif /* SMARTRECURSIVEPARALLELPREFIXSUM2PERFORMANCECOUNTERS_H_ */
