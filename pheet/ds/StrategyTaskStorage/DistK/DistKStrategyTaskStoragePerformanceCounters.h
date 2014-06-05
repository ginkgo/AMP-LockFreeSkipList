/*
 * DistKStrategyTaskStoragePerformanceCounters.h
 *
 *  Created on: 15.01.2013
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef DISTKSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_H_
#define DISTKSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Max/MaxPerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Time/TimePerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Time/MaxTimePerformanceCounter.h>

namespace pheet {

template <class Pheet, class StrategyHeapPerformanceCounters>
class DistKStrategyTaskStoragePerformanceCounters {
public:
	typedef DistKStrategyTaskStoragePerformanceCounters<Pheet, StrategyHeapPerformanceCounters> Self;

	inline DistKStrategyTaskStoragePerformanceCounters() {}
	inline DistKStrategyTaskStoragePerformanceCounters(Self& other)
	:num_blocks_created(other.num_blocks_created),
	 num_global_blocks(other.num_global_blocks),
	 num_unsuccessful_takes(other.num_unsuccessful_takes),
	 num_successful_takes(other.num_successful_takes),
	 num_taken_heap_items(other.num_taken_heap_items),
	 num_spied_tasks(other.num_spied_tasks),
	 num_inspected_global_items(other.num_inspected_global_items),
	 max_inspected_global_blocks(other.max_inspected_global_blocks),
	 make_global_time(other.make_global_time),
	 process_global_list_time(other.process_global_list_time),
	 process_global_list_max_time(other.process_global_list_max_time),
	 process_local_list_time(other.process_local_list_time),
	 try_connect_time(other.try_connect_time),
	 strategy_heap_performance_counters(other.strategy_heap_performance_counters) {}

	inline ~DistKStrategyTaskStoragePerformanceCounters() {}

	inline static void print_headers() {
		BasicPerformanceCounter<Pheet, task_storage_count_blocks_created>::print_header("num_blocks_created\t");
		BasicPerformanceCounter<Pheet, task_storage_count_global_blocks>::print_header("num_global_blocks\t");
		BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes>::print_header("num_unsuccessful_takes\t");
		BasicPerformanceCounter<Pheet, task_storage_count_successful_takes>::print_header("num_successful_takes\t");
		BasicPerformanceCounter<Pheet, task_storage_count_taken_heap_items>::print_header("num_taken_heap_items\t");
		BasicPerformanceCounter<Pheet, task_storage_count_spied_tasks>::print_header("num_spied_tasks\t");
		BasicPerformanceCounter<Pheet, task_storage_count_inspected_global_items>::print_header("num_inspected_global_items\t");
		MaxPerformanceCounter<Pheet, size_t, task_storage_count_max_inspected_global_blocks>::print_header("max_inspected_global_blocks\t");
		TimePerformanceCounter<Pheet, task_storage_measure_make_global_time>::print_header("make_global_time\t");
		TimePerformanceCounter<Pheet, task_storage_measure_process_global_list_time>::print_header("process_global_list_time\t");
		TimePerformanceCounter<Pheet, task_storage_measure_process_global_list_max_time>::print_header("process_global_list_max_time\t");
		TimePerformanceCounter<Pheet, task_storage_measure_process_local_list_time>::print_header("process_local_list_time\t");
		TimePerformanceCounter<Pheet, task_storage_measure_try_connect_time>::print_header("try_connect_time\t");

		StrategyHeapPerformanceCounters::print_headers();
	}
	inline void print_values() {
		num_blocks_created.print("%d\t");
		num_global_blocks.print("%d\t");
		num_unsuccessful_takes.print("%d\t");
		num_successful_takes.print("%d\t");
		num_taken_heap_items.print("%d\t");
		num_spied_tasks.print("%d\t");
		num_inspected_global_items.print("%d\t");
		max_inspected_global_blocks.print("%d\t");
		make_global_time.print("%f\t");
		process_global_list_time.print("%f\t");
		process_global_list_max_time.print("%f\t");
		process_local_list_time.print("%f\t");
		try_connect_time.print("%f\t");

		strategy_heap_performance_counters.print_values();
	}

	BasicPerformanceCounter<Pheet, task_storage_count_blocks_created> num_blocks_created;
	BasicPerformanceCounter<Pheet, task_storage_count_global_blocks> num_global_blocks;
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes> num_unsuccessful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_successful_takes> num_successful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_taken_heap_items> num_taken_heap_items;
	BasicPerformanceCounter<Pheet, task_storage_count_spied_tasks> num_spied_tasks;
	BasicPerformanceCounter<Pheet, task_storage_count_inspected_global_items> num_inspected_global_items;
	MaxPerformanceCounter<Pheet, size_t, task_storage_count_max_inspected_global_blocks> max_inspected_global_blocks;

	TimePerformanceCounter<Pheet, task_storage_measure_make_global_time> make_global_time;
	TimePerformanceCounter<Pheet, task_storage_measure_process_global_list_time> process_global_list_time;
	MaxTimePerformanceCounter<Pheet, task_storage_measure_process_global_list_max_time> process_global_list_max_time;
	TimePerformanceCounter<Pheet, task_storage_measure_process_local_list_time> process_local_list_time;
	TimePerformanceCounter<Pheet, task_storage_measure_try_connect_time> try_connect_time;

	StrategyHeapPerformanceCounters strategy_heap_performance_counters;
};

}

#endif /* DISTKSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_H_ */
