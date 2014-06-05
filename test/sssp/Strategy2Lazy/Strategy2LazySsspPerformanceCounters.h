/*
 * Strategy2LazySsspPerformanceCounters.h
 *
 *  Created on: Oct 08, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGY2LAZYSSSPPERFORMANCECOUNTERS_H_
#define STRATEGY2LAZYSSSPPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Time/LastTimePerformanceCounter.h>

namespace pheet {

template <class Pheet>
class Strategy2LazySsspPerformanceCounters {
public:
	typedef Strategy2LazySsspPerformanceCounters<Pheet> Self;

	Strategy2LazySsspPerformanceCounters() {}
	Strategy2LazySsspPerformanceCounters(Self& other)
	:num_dead_tasks(other.num_dead_tasks),
	 num_actual_tasks(other.num_actual_tasks),
	 last_non_dead_time(other.last_non_dead_time),
	 last_task_time(other.last_task_time),
	 last_update_time(other.last_update_time){}
	Strategy2LazySsspPerformanceCounters(Self&& other)
	:num_dead_tasks(other.num_dead_tasks),
	 num_actual_tasks(other.num_actual_tasks),
	 last_non_dead_time(other.last_non_dead_time),
	 last_task_time(other.last_task_time),
	 last_update_time(other.last_update_time) {}
	~Strategy2LazySsspPerformanceCounters() {}

	static void print_headers() {
		BasicPerformanceCounter<Pheet, sssp_count_dead_tasks>::print_header("num_dead_tasks\t");
		BasicPerformanceCounter<Pheet, sssp_count_actual_tasks>::print_header("num_actual_tasks\t");
		LastTimePerformanceCounter<Pheet, sssp_measure_last_non_dead_time>::print_header("last_non_dead_time\t");
		LastTimePerformanceCounter<Pheet, sssp_measure_last_task_time>::print_header("last_task_time\t");
		LastTimePerformanceCounter<Pheet, sssp_measure_last_update_time>::print_header("last_update_time\t");
	}
	void print_values() {
		num_dead_tasks.print("%d\t");
		num_actual_tasks.print("%d\t");
		last_non_dead_time.print("%f\t");
		last_task_time.print("%f\t");
		last_update_time.print("%f\t");
	}

	BasicPerformanceCounter<Pheet, sssp_count_dead_tasks> num_dead_tasks;
	BasicPerformanceCounter<Pheet, sssp_count_actual_tasks> num_actual_tasks;
	LastTimePerformanceCounter<Pheet, sssp_measure_last_non_dead_time> last_non_dead_time;
	LastTimePerformanceCounter<Pheet, sssp_measure_last_task_time> last_task_time;
	LastTimePerformanceCounter<Pheet, sssp_measure_last_update_time> last_update_time;
};

} /* namespace context */
#endif /* STRATEGY2LAZYSSSPPERFORMANCECOUNTERS_H_ */
