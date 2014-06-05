/*
 * ArrayListPrimaryTaskStoragePerformanceCounters.h
 *
 *  Created on: Nov 24, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LINKEDARRAYLISTPRIMARYTASKSTORAGEPERFORMANCECOUNTERS_H_
#define LINKEDARRAYLISTPRIMARYTASKSTORAGEPERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet>
class ArrayListPrimaryTaskStoragePerformanceCounters {
public:
	ArrayListPrimaryTaskStoragePerformanceCounters();
	ArrayListPrimaryTaskStoragePerformanceCounters(ArrayListPrimaryTaskStoragePerformanceCounters& other);
	~ArrayListPrimaryTaskStoragePerformanceCounters();

	void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_pops> num_unsuccessful_pops;
	BasicPerformanceCounter<Pheet, task_storage_count_successful_pops> num_successful_pops;
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes> num_unsuccessful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_successful_takes> num_successful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_size_pop> total_size_pop;
	TimePerformanceCounter<Pheet, task_storage_measure_pop_time> pop_time;
	TimePerformanceCounter<Pheet, task_storage_measure_push_time> push_time;
	BasicPerformanceCounter<Pheet, task_storage_count_skipped_cleanups> num_skipped_cleanups;
	MaxPerformanceCounter<Pheet, size_t, task_storage_measure_max_control_block_items> max_control_block_items;
};

template <class Pheet>
inline ArrayListPrimaryTaskStoragePerformanceCounters<Pheet>::ArrayListPrimaryTaskStoragePerformanceCounters() {

}

template <class Pheet>
ArrayListPrimaryTaskStoragePerformanceCounters<Pheet>::ArrayListPrimaryTaskStoragePerformanceCounters(ArrayListPrimaryTaskStoragePerformanceCounters& other)
:num_unsuccessful_pops(other.num_unsuccessful_pops),
 num_successful_pops(other.num_successful_pops),
 num_unsuccessful_takes(other.num_unsuccessful_takes),
 num_successful_takes(other.num_successful_takes),
 total_size_pop(other.total_size_pop),
 pop_time(other.pop_time),
 push_time(other.push_time),
 num_skipped_cleanups(other.num_skipped_cleanups),
 max_control_block_items(other.max_control_block_items)
{

}

template <class Pheet>
inline ArrayListPrimaryTaskStoragePerformanceCounters<Pheet>::~ArrayListPrimaryTaskStoragePerformanceCounters() {

}

template <class Pheet>
void ArrayListPrimaryTaskStoragePerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_pops>::print_header("num_unsuccessful_pops\t");
	BasicPerformanceCounter<Pheet, task_storage_count_successful_pops>::print_header("num_successful_pops\t");
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes>::print_header("num_unsuccessful_takes\t");
	BasicPerformanceCounter<Pheet, task_storage_count_successful_takes>::print_header("num_successful_takes\t");
	BasicPerformanceCounter<Pheet, task_storage_count_size_pop>::print_header("total_size_pop\t");
	TimePerformanceCounter<Pheet, task_storage_measure_pop_time>::print_header("pop_time\t");
	TimePerformanceCounter<Pheet, task_storage_measure_push_time>::print_header("push_time\t");
	TimePerformanceCounter<Pheet, task_storage_count_skipped_cleanups>::print_header("num_skipped_cleanups\t");
	MaxPerformanceCounter<Pheet, size_t, task_storage_measure_max_control_block_items>::print_header("max_control_block_items\t");
}

template <class Pheet>
void ArrayListPrimaryTaskStoragePerformanceCounters<Pheet>::print_values() {
	num_unsuccessful_pops.print("%d\t");
	num_successful_pops.print("%d\t");
	num_unsuccessful_takes.print("%d\t");
	num_successful_takes.print("%d\t");
	total_size_pop.print("%d\t");
	pop_time.print("%f\t");
	push_time.print("%f\t");
	num_skipped_cleanups.print("%d\t");
	max_control_block_items.print("%d\t");
}

}

#endif /* LINKEDARRAYLISTPRIMARYTASKSTORAGEPERFORMANCECOUNTERS_H_ */
