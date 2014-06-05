/*
 * BasicSchedulerPerformanceCounters.h
 *
 *  Created on: Nov 15, 2011
 *      Author: Martin Wimmer
 */

#ifndef BASICSCHEDULERPERFORMANCECOUNTERS_H_
#define BASICSCHEDULERPERFORMANCECOUNTERS_H_

#include "../../settings.h"

#include "../../primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "../../primitives/PerformanceCounter/Max/MaxPerformanceCounter.h"
#include "../../primitives/PerformanceCounter/Min/MinPerformanceCounter.h"
#include "../../primitives/PerformanceCounter/Time/TimePerformanceCounter.h"

namespace pheet {

template <class Pheet, class StealingDequePerformanceCounters, class FinishStackPerformanceCounters>
class BasicSchedulerPerformanceCounters {
public:
	typedef BasicSchedulerPerformanceCounters<Pheet, StealingDequePerformanceCounters, FinishStackPerformanceCounters> Self;

	BasicSchedulerPerformanceCounters() {}
	BasicSchedulerPerformanceCounters(Self& other)
		: num_spawns(other.num_spawns), num_actual_spawns(other.num_actual_spawns),
		  num_spawns_to_call(other.num_spawns_to_call),
		  num_calls(other.num_calls), num_finishes(other.num_finishes),
//		  num_completion_signals(other.num_completion_signals),
//		  num_chained_completion_signals(other.num_chained_completion_signals),
//		  num_remote_chained_completion_signals(other.num_remote_chained_completion_signals),
//		  num_non_blocking_finish_regions(other.num_non_blocking_finish_regions),
		  num_steals(other.num_steals), num_steal_calls(other.num_steal_calls),
		  num_unsuccessful_steal_calls(other.num_unsuccessful_steal_calls),
		  num_stealing_deque_pop_cas(other.num_stealing_deque_pop_cas),
		  num_dequeued_tasks(other.num_dequeued_tasks),
		  num_steal_executed_tasks(other.num_steal_executed_tasks),
		  total_time(other.total_time), task_time(other.task_time),
		  idle_time(other.idle_time),
//		  finish_stack_nonblocking_max(other.finish_stack_nonblocking_max),
//		  finish_stack_blocking_min(other.finish_stack_blocking_min),
		  stealing_deque_performance_counters(other.stealing_deque_performance_counters),
		  finish_stack_performance_counters(other.finish_stack_performance_counters) {}

	static void print_headers();
	void print_values();

//private:
	BasicPerformanceCounter<Pheet, scheduler_count_spawns> num_spawns;
	BasicPerformanceCounter<Pheet, scheduler_count_actual_spawns> num_actual_spawns;
	BasicPerformanceCounter<Pheet, scheduler_count_spawns_to_call> num_spawns_to_call;
	BasicPerformanceCounter<Pheet, scheduler_count_calls> num_calls;
	BasicPerformanceCounter<Pheet, scheduler_count_finishes> num_finishes;
//	BasicPerformanceCounter<Pheet, scheduler_count_completion_signals> num_completion_signals;
//	BasicPerformanceCounter<Pheet, scheduler_count_chained_completion_signals> num_chained_completion_signals;
//	BasicPerformanceCounter<Pheet, scheduler_count_remote_chained_completion_signals> num_remote_chained_completion_signals;
//	BasicPerformanceCounter<Pheet, scheduler_count_non_blocking_finish_regions> num_non_blocking_finish_regions;

	BasicPerformanceCounter<Pheet, task_storage_count_steals> num_steals;
	BasicPerformanceCounter<Pheet, task_storage_count_steal_calls> num_steal_calls;
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_steal_calls> num_unsuccessful_steal_calls;
	BasicPerformanceCounter<Pheet, task_storage_count_pop_cas> num_stealing_deque_pop_cas;

	BasicPerformanceCounter<Pheet, task_storage_count_dequeued_tasks> num_dequeued_tasks;
	BasicPerformanceCounter<Pheet, task_storage_count_steal_executed_tasks> num_steal_executed_tasks;

	TimePerformanceCounter<Pheet, scheduler_measure_total_time> total_time;
	TimePerformanceCounter<Pheet, scheduler_measure_task_time> task_time;
	TimePerformanceCounter<Pheet, scheduler_measure_idle_time> idle_time;

//	MaxPerformanceCounter<Pheet, size_t, scheduler_measure_finish_stack_nonblocking_max> finish_stack_nonblocking_max;
//	MinPerformanceCounter<Pheet, size_t, scheduler_measure_finish_stack_blocking_min> finish_stack_blocking_min;

	StealingDequePerformanceCounters stealing_deque_performance_counters;
	FinishStackPerformanceCounters finish_stack_performance_counters;
};

template <class Pheet, class StealingDequePerformanceCounters, class FinishStackPerformanceCounters>
inline void BasicSchedulerPerformanceCounters<Pheet, StealingDequePerformanceCounters, FinishStackPerformanceCounters>::print_headers() {
	BasicPerformanceCounter<Pheet, scheduler_count_spawns>::print_header("spawns\t");
	BasicPerformanceCounter<Pheet, scheduler_count_actual_spawns>::print_header("actual_spawns\t");
	BasicPerformanceCounter<Pheet, scheduler_count_spawns_to_call>::print_header("calls\t");
	BasicPerformanceCounter<Pheet, scheduler_count_calls>::print_header("spawns->call\t");
	BasicPerformanceCounter<Pheet, scheduler_count_finishes>::print_header("finishes\t");
//	BasicPerformanceCounter<Pheet, scheduler_count_completion_signals>::print_header("num_completion_signals\t");
//	BasicPerformanceCounter<Pheet, scheduler_count_chained_completion_signals>::print_header("num_chained_completion_signals\t");
//	BasicPerformanceCounter<Pheet, scheduler_count_remote_chained_completion_signals>::print_header("num_remote_chained_completion_signals\t");
//	BasicPerformanceCounter<Pheet, scheduler_count_non_blocking_finish_regions>::print_header("num_non_blocking_finish_regions\t");

	BasicPerformanceCounter<Pheet, task_storage_count_steals>::print_header("stolen\t");
	BasicPerformanceCounter<Pheet, task_storage_count_steal_calls>::print_header("steal_calls\t");
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_steal_calls>::print_header("unsuccessful_steal_calls\t");
	BasicPerformanceCounter<Pheet, task_storage_count_pop_cas>::print_header("stealing_deque_pop_cas\t");

	BasicPerformanceCounter<Pheet, task_storage_count_dequeued_tasks>::print_header("num_dequeued_tasks\t");
	BasicPerformanceCounter<Pheet, task_storage_count_steal_executed_tasks>::print_header("num_steal_executed_tasks\t");

	TimePerformanceCounter<Pheet, scheduler_measure_total_time>::print_header("scheduler_total_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_task_time>::print_header("total_task_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_idle_time>::print_header("total_idle_time\t");

//	MaxPerformanceCounter<Pheet, size_t, scheduler_measure_finish_stack_nonblocking_max>::print_header("finish_stack_nonblocking_max\t");
//	MinPerformanceCounter<Pheet, size_t, scheduler_measure_finish_stack_blocking_min>::print_header("finish_stack_blocking_min\t");

	StealingDequePerformanceCounters::print_headers();
	FinishStackPerformanceCounters::print_headers();
}

template <class Pheet, class StealingDequePerformanceCounters, class FinishStackPerformanceCounters>
inline void BasicSchedulerPerformanceCounters<Pheet, StealingDequePerformanceCounters, FinishStackPerformanceCounters>::print_values() {
	num_spawns.print("%lu\t");
	num_actual_spawns.print("%lu\t");
	num_calls.print("%lu\t");
	num_spawns_to_call.print("%lu\t");
	num_finishes.print("%lu\t");
//	num_completion_signals.print("%lu\t");
//	num_chained_completion_signals.print("%lu\t");
//	num_remote_chained_completion_signals.print("%lu\t");
//	num_non_blocking_finish_regions.print("%lu\t");
	num_steals.print("%lu\t");
	num_steal_calls.print("%lu\t");
	num_unsuccessful_steal_calls.print("%lu\t");
	num_stealing_deque_pop_cas.print("%lu\t");
	num_dequeued_tasks.print("%lu\t");
	num_steal_executed_tasks.print("%lu\t");
	total_time.print("%f\t");
	task_time.print("%f\t");
	idle_time.print("%f\t");

//	finish_stack_nonblocking_max.print("%lu\t");
//	finish_stack_blocking_min.print("%lu\t");

	stealing_deque_performance_counters.print_values();
	finish_stack_performance_counters.print_values();
}

}

#endif /* BASICSCHEDULERPERFORMANCECOUNTERS_H_ */
