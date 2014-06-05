/*
 * MMFinishStackPerformanceCounters.h
 *
 *  Created on: Jul 30, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef MMFINISHSTACKPERFORMANCECOUNTERS_H_
#define MMFINISHSTACKPERFORMANCECOUNTERS_H_

#include <pheet/settings.h>

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Max/MaxPerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Min/MinPerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Time/TimePerformanceCounter.h>

namespace pheet {

template <class Pheet>
class MMFinishStackPerformanceCounters {
public:
	typedef MMFinishStackPerformanceCounters<Pheet> Self;

	MMFinishStackPerformanceCounters() {}
	MMFinishStackPerformanceCounters(Self& other)
		: num_completion_signals(other.num_completion_signals),
		  num_chained_completion_signals(other.num_chained_completion_signals),
		  num_remote_chained_completion_signals(other.num_remote_chained_completion_signals),
		  num_non_blocking_finish_regions(other.num_non_blocking_finish_regions),
		  num_finish_stack_mm_accesses(other.num_finish_stack_mm_accesses) {}

	static void print_headers();
	void print_values();

//private:
	BasicPerformanceCounter<Pheet, finish_stack_count_completion_signals> num_completion_signals;
	BasicPerformanceCounter<Pheet, finish_stack_count_chained_completion_signals> num_chained_completion_signals;
	BasicPerformanceCounter<Pheet, finish_stack_count_remote_chained_completion_signals> num_remote_chained_completion_signals;
	BasicPerformanceCounter<Pheet, finish_stack_count_non_blocking_finish_regions> num_non_blocking_finish_regions;
	BasicPerformanceCounter<Pheet, finish_stack_count_mm_accesses> num_finish_stack_mm_accesses;
};

template <class Pheet>
inline void MMFinishStackPerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, finish_stack_count_completion_signals>::print_header("num_completion_signals\t");
	BasicPerformanceCounter<Pheet, finish_stack_count_chained_completion_signals>::print_header("num_chained_completion_signals\t");
	BasicPerformanceCounter<Pheet, finish_stack_count_remote_chained_completion_signals>::print_header("num_remote_chained_completion_signals\t");
	BasicPerformanceCounter<Pheet, finish_stack_count_non_blocking_finish_regions>::print_header("num_non_blocking_finish_regions\t");
	BasicPerformanceCounter<Pheet, finish_stack_count_mm_accesses>::print_header("num_finish_stack_mm_accesses\t");
}

template <class Pheet>
inline void MMFinishStackPerformanceCounters<Pheet>::print_values() {
	num_completion_signals.print("%lu\t");
	num_chained_completion_signals.print("%lu\t");
	num_remote_chained_completion_signals.print("%lu\t");
	num_non_blocking_finish_regions.print("%lu\t");
	num_finish_stack_mm_accesses.print("%lu\t");
}

} /* namespace pheet */
#endif /* MMFINISHSTACKPERFORMANCECOUNTERS_H_ */
