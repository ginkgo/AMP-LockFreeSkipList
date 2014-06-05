/*
 * StrategySchedulerPerformanceCounters.h
 *
 *  Created on: Oct 28, 2011
 *      Author: Martin Wimmer
 */

#ifndef STRATEGYSCHEDULERPERFORMANCECOUNTERS_H_
#define STRATEGYSCHEDULERPERFORMANCECOUNTERS_H_

#include "../../settings.h"

#include "../../primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "../../primitives/PerformanceCounter/Max/MaxPerformanceCounter.h"
#include "../../primitives/PerformanceCounter/Min/MinPerformanceCounter.h"
#include "../../primitives/PerformanceCounter/Time/TimePerformanceCounter.h"

namespace pheet {

template <class Pheet, class TaskStoragePerformanceCounters, class StealerPerformanceCounters, class FinishStackPerformanceCounters>
class StrategySchedulerPerformanceCounters {
public:
	StrategySchedulerPerformanceCounters() {}
	StrategySchedulerPerformanceCounters(StrategySchedulerPerformanceCounters<Pheet, TaskStoragePerformanceCounters, StealerPerformanceCounters, FinishStackPerformanceCounters>& other)
		: num_spawns(other.num_spawns), num_actual_spawns(other.num_actual_spawns),
		  num_spawns_to_call(other.num_spawns_to_call),
		  num_calls(other.num_calls), num_finishes(other.num_finishes),
		  num_steal_calls(other.num_steal_calls),
		  num_unsuccessful_steal_calls(other.num_unsuccessful_steal_calls),
		  total_time(other.total_time), task_time(other.task_time),
		  idle_time(other.idle_time), steal_time(other.steal_time),
		  finish_stack_nonblocking_max(other.finish_stack_nonblocking_max),
		  finish_stack_blocking_min(other.finish_stack_blocking_min),
		  task_storage_performance_counters(other.task_storage_performance_counters),
		  stealer_performance_counters(other.stealer_performance_counters),
		  finish_stack_performance_counters(other.finish_stack_performance_counters) {}

	static void print_headers();
	void print_values();

//private:
	BasicPerformanceCounter<Pheet, scheduler_count_spawns> num_spawns;
	BasicPerformanceCounter<Pheet, scheduler_count_actual_spawns> num_actual_spawns;
	BasicPerformanceCounter<Pheet, scheduler_count_spawns_to_call> num_spawns_to_call;
	BasicPerformanceCounter<Pheet, scheduler_count_calls> num_calls;
	BasicPerformanceCounter<Pheet, scheduler_count_finishes> num_finishes;

	BasicPerformanceCounter<Pheet, task_storage_count_steal_calls> num_steal_calls;
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_steal_calls> num_unsuccessful_steal_calls;

	TimePerformanceCounter<Pheet, scheduler_measure_total_time> total_time;
	TimePerformanceCounter<Pheet, scheduler_measure_task_time> task_time;
	TimePerformanceCounter<Pheet, scheduler_measure_idle_time> idle_time;
	TimePerformanceCounter<Pheet, scheduler_measure_idle_time> steal_time;

	MaxPerformanceCounter<Pheet, size_t, finish_stack_track_nonblocking_max> finish_stack_nonblocking_max;
	MinPerformanceCounter<Pheet, size_t, finish_stack_track_blocking_min> finish_stack_blocking_min;

	TaskStoragePerformanceCounters task_storage_performance_counters;
	StealerPerformanceCounters stealer_performance_counters;
	FinishStackPerformanceCounters finish_stack_performance_counters;
};

template <class Pheet, class TaskStoragePerformanceCounters, class StealerPerformanceCounters, class FinishStackPerformanceCounters>
inline void StrategySchedulerPerformanceCounters<Pheet, TaskStoragePerformanceCounters, StealerPerformanceCounters, FinishStackPerformanceCounters>::print_headers() {
	BasicPerformanceCounter<Pheet, scheduler_count_spawns>::print_header("spawns\t");
	BasicPerformanceCounter<Pheet, scheduler_count_actual_spawns>::print_header("actual_spawns\t");
	BasicPerformanceCounter<Pheet, scheduler_count_calls>::print_header("calls\t");
	BasicPerformanceCounter<Pheet, scheduler_count_spawns_to_call>::print_header("spawns->call\t");
	BasicPerformanceCounter<Pheet, scheduler_count_finishes>::print_header("finishes\t");

	BasicPerformanceCounter<Pheet, task_storage_count_steal_calls>::print_header("steal_calls\t");
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_steal_calls>::print_header("unsuccessful_steal_calls\t");

	TimePerformanceCounter<Pheet, scheduler_measure_total_time>::print_header("scheduler_total_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_task_time>::print_header("total_task_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_idle_time>::print_header("total_idle_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_steal_time>::print_header("total_steal_time\t");

	MaxPerformanceCounter<Pheet, size_t, finish_stack_track_nonblocking_max>::print_header("finish_stack_nonblocking_max\t");
	MinPerformanceCounter<Pheet, size_t, finish_stack_track_blocking_min>::print_header("finish_stack_blocking_min\t");

	TaskStoragePerformanceCounters::print_headers();
	StealerPerformanceCounters::print_headers();
	FinishStackPerformanceCounters::print_headers();
}

template <class Pheet, class TaskStoragePerformanceCounters, class StealerPerformanceCounters, class FinishStackPerformanceCounters>
inline void StrategySchedulerPerformanceCounters<Pheet, TaskStoragePerformanceCounters, StealerPerformanceCounters, FinishStackPerformanceCounters>::print_values() {
	num_spawns.print("%lu\t");
	num_actual_spawns.print("%lu\t");
	num_calls.print("%lu\t");
	num_spawns_to_call.print("%lu\t");
	num_finishes.print("%lu\t");
	num_steal_calls.print("%lu\t");
	num_unsuccessful_steal_calls.print("%lu\t");
	total_time.print("%f\t");
	task_time.print("%f\t");
	idle_time.print("%f\t");
	steal_time.print("%f\t");

	finish_stack_nonblocking_max.print("%lu\t");
	finish_stack_blocking_min.print("%lu\t");

	task_storage_performance_counters.print_values();
	stealer_performance_counters.print_values();
	finish_stack_performance_counters.print_values();
}

}

#endif /* STRATEGYSCHEDULERPERFORMANCECOUNTERS_H_ */
