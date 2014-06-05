/*
 * ArrayListHeapPrimaryTaskStoragePerformanceCounters.h
 *
 *  Created on: Dec 7, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ARRAYLISTHEAPPRIMARYTASKSTORAGEPERFORMANCECOUNTERS_H_
#define ARRAYLISTHEAPPRIMARYTASKSTORAGEPERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet>
class ArrayListHeapPrimaryTaskStoragePerformanceCounters {
public:
	ArrayListHeapPrimaryTaskStoragePerformanceCounters();
	ArrayListHeapPrimaryTaskStoragePerformanceCounters(ArrayListHeapPrimaryTaskStoragePerformanceCounters& other);
	~ArrayListHeapPrimaryTaskStoragePerformanceCounters();

	static void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_pops> num_unsuccessful_pops;
	BasicPerformanceCounter<Pheet, task_storage_count_successful_pops> num_successful_pops;
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes> num_unsuccessful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_successful_takes> num_successful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_size_pop> total_size_pop;
	TimePerformanceCounter<Pheet, task_storage_measure_pop_time> pop_time;
	TimePerformanceCounter<Pheet, task_storage_measure_push_time> push_time;
	TimePerformanceCounter<Pheet, task_storage_measure_clean_time> clean_time;
	TimePerformanceCounter<Pheet, task_storage_measure_create_control_block_time> create_control_block_time;
	TimePerformanceCounter<Pheet, task_storage_measure_configure_successor_time> configure_successor_time;
	TimePerformanceCounter<Pheet, task_storage_measure_heap_push_time> heap_push_time;
	TimePerformanceCounter<Pheet, task_storage_measure_put_time> put_time;
	TimePerformanceCounter<Pheet, task_storage_measure_strategy_alloc_time> strategy_alloc_time;
	BasicPerformanceCounter<Pheet, task_storage_count_skipped_cleanups> num_skipped_cleanups;
	MaxPerformanceCounter<Pheet, size_t, task_storage_track_max_control_block_items> max_control_block_items;
	MaxPerformanceCounter<Pheet, size_t, task_storage_track_max_heap_length> max_heap_length;
};

template <class Pheet>
inline ArrayListHeapPrimaryTaskStoragePerformanceCounters<Pheet>::ArrayListHeapPrimaryTaskStoragePerformanceCounters() {

}

template <class Pheet>
ArrayListHeapPrimaryTaskStoragePerformanceCounters<Pheet>::ArrayListHeapPrimaryTaskStoragePerformanceCounters(ArrayListHeapPrimaryTaskStoragePerformanceCounters& other)
:num_unsuccessful_pops(other.num_unsuccessful_pops),
 num_successful_pops(other.num_successful_pops),
 num_unsuccessful_takes(other.num_unsuccessful_takes),
 num_successful_takes(other.num_successful_takes),
 total_size_pop(other.total_size_pop),
 pop_time(other.pop_time),
 push_time(other.push_time),
 clean_time(other.clean_time),
 create_control_block_time(other.create_control_block_time),
 configure_successor_time(other.configure_successor_time),
 heap_push_time(other.heap_push_time),
 put_time(other.put_time),
 strategy_alloc_time(other.strategy_alloc_time),
 num_skipped_cleanups(other.num_skipped_cleanups),
 max_control_block_items(other.max_control_block_items),
 max_heap_length(other.max_heap_length)
{

}

template <class Pheet>
inline ArrayListHeapPrimaryTaskStoragePerformanceCounters<Pheet>::~ArrayListHeapPrimaryTaskStoragePerformanceCounters() {

}

template <class Pheet>
void ArrayListHeapPrimaryTaskStoragePerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_pops>::print_header("num_unsuccessful_pops\t");
	BasicPerformanceCounter<Pheet, task_storage_count_successful_pops>::print_header("num_successful_pops\t");
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes>::print_header("num_unsuccessful_takes\t");
	BasicPerformanceCounter<Pheet, task_storage_count_successful_takes>::print_header("num_successful_takes\t");
	BasicPerformanceCounter<Pheet, task_storage_count_size_pop>::print_header("total_size_pop\t");
	TimePerformanceCounter<Pheet, task_storage_measure_pop_time>::print_header("pop_time\t");
	TimePerformanceCounter<Pheet, task_storage_measure_push_time>::print_header("push_time\t");
	TimePerformanceCounter<Pheet, task_storage_measure_clean_time>::print_header("clean_time\t");
	TimePerformanceCounter<Pheet, task_storage_measure_create_control_block_time>::print_header("create_control_block_time\t");
	TimePerformanceCounter<Pheet, task_storage_measure_configure_successor_time>::print_header("configure_successor_time\t");
	TimePerformanceCounter<Pheet, task_storage_measure_heap_push_time>::print_header("heap_push_time\t");
	TimePerformanceCounter<Pheet, task_storage_measure_put_time>::print_header("put_time\t");
	TimePerformanceCounter<Pheet, task_storage_measure_strategy_alloc_time>::print_header("strategy_alloc_time\t");
	TimePerformanceCounter<Pheet, task_storage_count_skipped_cleanups>::print_header("num_skipped_cleanups\t");
	MaxPerformanceCounter<Pheet, size_t, task_storage_track_max_control_block_items>::print_header("max_control_block_items\t");
	MaxPerformanceCounter<Pheet, size_t, task_storage_track_max_heap_length>::print_header("max_heap_length\t");
}

template <class Pheet>
void ArrayListHeapPrimaryTaskStoragePerformanceCounters<Pheet>::print_values() {
	num_unsuccessful_pops.print("%d\t");
	num_successful_pops.print("%d\t");
	num_unsuccessful_takes.print("%d\t");
	num_successful_takes.print("%d\t");
	total_size_pop.print("%d\t");
	pop_time.print("%f\t");
	push_time.print("%f\t");
	clean_time.print("%f\t");
	create_control_block_time.print("%f\t");
	configure_successor_time.print("%f\t");
	heap_push_time.print("%f\t");
	put_time.print("%f\t");
	strategy_alloc_time.print("%f\t");
	num_skipped_cleanups.print("%d\t");
	max_control_block_items.print("%d\t");
	max_heap_length.print("%d\t");
}

}

#endif /* ARRAYLISTHEAPPRIMARYTASKSTORAGEPERFORMANCECOUNTERS_H_ */
