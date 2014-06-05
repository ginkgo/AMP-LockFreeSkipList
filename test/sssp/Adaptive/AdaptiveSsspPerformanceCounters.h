/*
 * AdaptiveSsspPerformanceCounters.h
 *
 *  Created on: Jan 31, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef ADAPTIVESSSPPERFORMANCECOUNTERS_H_
#define ADAPTIVESSSPPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>

namespace pheet {

template <class Pheet>
class AdaptiveSsspPerformanceCounters {
public:
	typedef AdaptiveSsspPerformanceCounters<Pheet> Self;

	AdaptiveSsspPerformanceCounters() {}
	AdaptiveSsspPerformanceCounters(Self& other)
	:num_dead_tasks(other.num_dead_tasks),
	 num_actual_tasks(other.num_actual_tasks) {}
	AdaptiveSsspPerformanceCounters(Self&& other)
	:num_dead_tasks(other.num_dead_tasks),
	 num_actual_tasks(other.num_actual_tasks) {}
	~AdaptiveSsspPerformanceCounters() {}

	static void print_headers() {
		BasicPerformanceCounter<Pheet, sssp_count_dead_tasks>::print_header("num_dead_tasks\t");
		BasicPerformanceCounter<Pheet, sssp_count_actual_tasks>::print_header("num_actual_tasks\t");
	}
	void print_values() {
		num_dead_tasks.print("%d\t");
		num_actual_tasks.print("%d\t");
	}

	BasicPerformanceCounter<Pheet, sssp_count_dead_tasks> num_dead_tasks;
	BasicPerformanceCounter<Pheet, sssp_count_actual_tasks> num_actual_tasks;
};

} /* namespace context */
#endif /* ADAPTIVESSSPPERFORMANCECOUNTERS_H_ */
