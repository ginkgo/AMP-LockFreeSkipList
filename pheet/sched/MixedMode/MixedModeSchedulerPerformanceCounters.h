/*
 * MixedModeSchedulerPerformanceCounters.h
 *
 *  Created on: Mar 6, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICMIXEDMODESCHEDULERPERFORMANCECOUNTERS_H_
#define BASICMIXEDMODESCHEDULERPERFORMANCECOUNTERS_H_

#include "../../primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "../../primitives/PerformanceCounter/Basic/BasicPerformanceCounterVector.h"
#include "../../primitives/PerformanceCounter/Time/TimePerformanceCounter.h"

namespace pheet {

template <class Pheet, class StealingDequePerformanceCounters>
struct MixedModeSchedulerPerformanceCounters {
	MixedModeSchedulerPerformanceCounters() {}

	MixedModeSchedulerPerformanceCounters(MixedModeSchedulerPerformanceCounters& other)
		: /*num_tasks_at_level(other.num_tasks_at_level),
		  num_steal_calls_per_thread(other.num_steal_calls_per_thread),
		  num_unsuccessful_steal_calls_per_thread(other.num_unsuccessful_steal_calls_per_thread),*/
		  num_spawns(other.num_spawns), num_spawns_to_call(other.num_spawns_to_call),
		  num_calls(other.num_calls), num_finishes(other.num_finishes),
		  num_steals(other.num_steals), num_steal_calls(other.num_steal_calls),
		  num_unsuccessful_steal_calls(other.num_unsuccessful_steal_calls),
		  num_stealing_deque_pop_cas(other.num_stealing_deque_pop_cas),
		  total_time(other.total_time), task_time(other.task_time),
		  sync_time(other.sync_time), idle_time(other.idle_time),
		  queue_processing_time(other.queue_processing_time),
		  visit_partners_time(other.visit_partners_time),
		  wait_for_finish_time(other.wait_for_finish_time),
		  wait_for_coordinator_time(other.wait_for_coordinator_time),
		  stealing_deque_performance_counters(other.stealing_deque_performance_counters) {}

	static void print_headers();
	void print_values();

/*
	BasicPerformanceCounterVector<Pheet, scheduler_count_tasks_at_level> num_tasks_at_level;
	BasicPerformanceCounterVector<Pheet, scheduler_count_steal_calls_per_thread> num_steal_calls_per_thread;
	BasicPerformanceCounterVector<Pheet, scheduler_count_unsuccessful_steal_calls_per_thread> num_unsuccessful_steal_calls_per_thread;
*/
	BasicPerformanceCounter<Pheet, scheduler_count_spawns> num_spawns;
	BasicPerformanceCounter<Pheet, scheduler_count_spawns_to_call> num_spawns_to_call;
	BasicPerformanceCounter<Pheet, scheduler_count_calls> num_calls;
	BasicPerformanceCounter<Pheet, scheduler_count_finishes> num_finishes;

	BasicPerformanceCounter<Pheet, task_storage_count_steals> num_steals;
	BasicPerformanceCounter<Pheet, task_storage_count_steal_calls> num_steal_calls;
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_steal_calls> num_unsuccessful_steal_calls;
	BasicPerformanceCounter<Pheet, task_storage_count_pop_cas> num_stealing_deque_pop_cas;

	TimePerformanceCounter<Pheet, scheduler_measure_total_time> total_time;
	TimePerformanceCounter<Pheet, scheduler_measure_task_time> task_time;
	TimePerformanceCounter<Pheet, scheduler_measure_sync_time> sync_time;
	TimePerformanceCounter<Pheet, scheduler_measure_idle_time> idle_time;
	TimePerformanceCounter<Pheet, scheduler_measure_queue_processing_time> queue_processing_time;
	TimePerformanceCounter<Pheet, scheduler_measure_visit_partners_time> visit_partners_time;
	TimePerformanceCounter<Pheet, scheduler_measure_wait_for_finish_time> wait_for_finish_time;
	TimePerformanceCounter<Pheet, scheduler_measure_wait_for_coordinator_time> wait_for_coordinator_time;

	StealingDequePerformanceCounters stealing_deque_performance_counters;
};


template <class Pheet, class StealingDequePerformanceCounters>
inline void MixedModeSchedulerPerformanceCounters<Pheet, StealingDequePerformanceCounters>::print_headers() {
	BasicPerformanceCounter<Pheet, scheduler_count_spawns>::print_header("spawns\t");
	BasicPerformanceCounter<Pheet, scheduler_count_spawns_to_call>::print_header("calls\t");
	BasicPerformanceCounter<Pheet, scheduler_count_calls>::print_header("spawns->call\t");
	BasicPerformanceCounter<Pheet, scheduler_count_finishes>::print_header("finishes\t");

	BasicPerformanceCounter<Pheet, task_storage_count_steals>::print_header("stolen\t");
	BasicPerformanceCounter<Pheet, task_storage_count_steal_calls>::print_header("steal_calls\t");
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_steal_calls>::print_header("unsuccessful_steal_calls\t");
	BasicPerformanceCounter<Pheet, task_storage_count_pop_cas>::print_header("stealing_deque_pop_cas\t");

	TimePerformanceCounter<Pheet, scheduler_measure_total_time>::print_header("scheduler_total_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_task_time>::print_header("total_task_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_sync_time>::print_header("total_sync_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_idle_time>::print_header("total_idle_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_queue_processing_time>::print_header("queue_processing_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_visit_partners_time>::print_header("visit_partners_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_wait_for_finish_time>::print_header("wait_for_finish_time\t");
	TimePerformanceCounter<Pheet, scheduler_measure_wait_for_coordinator_time>::print_header("wait_for_coordinator_time\t");

	/*if(scheduler_count_tasks_at_level) {
		procs_t depth = cpu_hierarchy.get_max_depth();
	//	char header[32];
		for(procs_t i = 0; i < depth; ++i) {
			printf("tasks_at_level_%d\t", (int)i);
		}
	}
	if(scheduler_count_steal_calls_per_thread) {
		for(procs_t i = 0; i < num_places; ++i) {
			printf("steal_calls_at_thread_%d\t", (int)i);
		}
	}
	if(scheduler_count_unsuccessful_steal_calls_per_thread) {
		for(procs_t i = 0; i < num_places; ++i) {
			printf("unsuccessful_steal_calls_at_thread_%d\t", (int)i);
		}
	}*/

	StealingDequePerformanceCounters::print_headers();
}

template <class Pheet, class StealingDequePerformanceCounters>
inline void MixedModeSchedulerPerformanceCounters<Pheet, StealingDequePerformanceCounters>::print_values() {
	num_spawns.print("%d\t");
	num_calls.print("%d\t");
	num_spawns_to_call.print("%d\t");
	num_finishes.print("%d\t");
	num_steals.print("%d\t");
	num_steal_calls.print("%d\t");
	num_unsuccessful_steal_calls.print("%d\t");
	num_stealing_deque_pop_cas.print("%d\t");
	total_time.print("%f\t");
	task_time.print("%f\t");
	sync_time.print("%f\t");
	idle_time.print("%f\t");
	queue_processing_time.print("%f\t");
	visit_partners_time.print("%f\t");
	wait_for_finish_time.print("%f\t");
	wait_for_coordinator_time.print("%f\t");
/*
	if(scheduler_count_tasks_at_level) {
		procs_t depth = cpu_hierarchy.get_max_depth();
		for(procs_t i = 0; i < depth; ++i) {
			num_tasks_at_level.print(i, "%d\t");
		}
	}
	if(scheduler_count_steal_calls_per_thread) {
		for(procs_t i = 0; i < num_places; ++i) {
			num_steal_calls_per_thread.print(i, "%d\t");
		}
	}
	if(scheduler_count_unsuccessful_steal_calls_per_thread) {
		for(procs_t i = 0; i < num_places; ++i) {
			num_unsuccessful_steal_calls_per_thread.print(i, "%d\t");
		}
	}*/

	stealing_deque_performance_counters.print_values();
}



}

#endif /* BASICMIXEDMODESCHEDULERPERFORMANCECOUNTERS_H_ */
