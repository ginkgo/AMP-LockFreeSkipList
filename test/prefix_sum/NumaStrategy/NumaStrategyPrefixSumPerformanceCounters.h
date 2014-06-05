/*
 * NumaStrategyPrefixSumPerformanceCounters.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef NUMASTRATEGYPREFIXSUMPERFORMANCECOUNTERS_H_
#define NUMASTRATEGYPREFIXSUMPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Time/TimePerformanceCounter.h>
#include <iostream>

namespace pheet {

template <class Pheet>
class NumaStrategyPrefixSumPerformanceCounters {
public:
	typedef NumaStrategyPrefixSumPerformanceCounters<Pheet> Self;

	NumaStrategyPrefixSumPerformanceCounters() {}
	NumaStrategyPrefixSumPerformanceCounters(Self& other)
	: blocks(other.blocks), preprocessed_blocks(other.preprocessed_blocks),
	  numa_local_blocks(other.numa_local_blocks), numa_non_local_blocks(other.numa_non_local_blocks),
	  numa_cache_time(other.numa_cache_time) {}
	NumaStrategyPrefixSumPerformanceCounters(Self&& other)
	: blocks(other.blocks), preprocessed_blocks(other.preprocessed_blocks),
	  numa_local_blocks(other.numa_local_blocks), numa_non_local_blocks(other.numa_non_local_blocks),
	  numa_cache_time(other.numa_cache_time) {}
	~NumaStrategyPrefixSumPerformanceCounters() {}

	static void print_headers() {
		BasicPerformanceCounter<Pheet, prefix_sum_log_pf_blocks>::print_header("blocks\t");
		BasicPerformanceCounter<Pheet, prefix_sum_log_pf_preprocessed_blocks>::print_header("preprocessed_blocks\t");
		BasicPerformanceCounter<Pheet, prefix_sum_log_numa_local_blocks>::print_header("numa_local_blocks\t");
		BasicPerformanceCounter<Pheet, prefix_sum_log_numa_non_local_blocks>::print_header("numa_non_local_blocks\t");
		TimePerformanceCounter<Pheet, prefix_sum_log_numa_cache_time>::print_header("numa_cache_time\t");
	}
	void print_values() {
		blocks.print("%lu\t");
		preprocessed_blocks.print("%lu\t");
		numa_local_blocks.print("%lu\t");
		numa_non_local_blocks.print("%lu\t");
		numa_cache_time.print("%f\t");
	}

	BasicPerformanceCounter<Pheet, prefix_sum_log_pf_blocks> blocks;
	BasicPerformanceCounter<Pheet, prefix_sum_log_pf_preprocessed_blocks> preprocessed_blocks;
	BasicPerformanceCounter<Pheet, prefix_sum_log_numa_local_blocks> numa_local_blocks;
	BasicPerformanceCounter<Pheet, prefix_sum_log_numa_non_local_blocks> numa_non_local_blocks;
	TimePerformanceCounter<Pheet, prefix_sum_log_numa_cache_time> numa_cache_time;
};

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUMPERFORMANCECOUNTERS_H_ */
