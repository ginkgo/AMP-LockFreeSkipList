/*
 * Strategy2BaseTaskStoragePerformanceCounters.h
 *
 *  Created on: Sep 17, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGY2BASETASKSTORAGEPERFORMANCECOUNTERS_H_
#define STRATEGY2BASETASKSTORAGEPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>

namespace pheet {

template <class Pheet>
class Strategy2BaseTaskStoragePerformanceCounters {
public:
	typedef Strategy2BaseTaskStoragePerformanceCounters<Pheet> Self;

	Strategy2BaseTaskStoragePerformanceCounters() {}
	Strategy2BaseTaskStoragePerformanceCounters(Self& other)
	:num_blocks_created(other.num_blocks_created),
	 num_global_blocks(other.num_global_blocks),
	 num_inspected_global_items(other.num_inspected_global_items),
	 num_spied_tasks(other.num_spied_tasks),
	 num_spied_global_tasks(other.num_spied_global_tasks),
	 num_merges(other.num_merges),
	 num_allocated_items(other.num_allocated_items) {}

	~Strategy2BaseTaskStoragePerformanceCounters() {}

	inline static void print_headers() {
		BasicPerformanceCounter<Pheet, task_storage_count_blocks_created>::print_header("num_blocks_created\t");
		BasicPerformanceCounter<Pheet, task_storage_count_global_blocks>::print_header("num_global_blocks\t");
		BasicPerformanceCounter<Pheet, task_storage_count_inspected_global_items>::print_header("num_inspected_global_items\t");
		BasicPerformanceCounter<Pheet, task_storage_count_spied_tasks>::print_header("num_spied_tasks\t");
		BasicPerformanceCounter<Pheet, task_storage_count_spied_global_tasks>::print_header("num_spied_global_tasks\t");
		BasicPerformanceCounter<Pheet, task_storage_count_merges>::print_header("num_merges\t");
		BasicPerformanceCounter<Pheet, task_storage_count_allocated_items>::print_header("num_allocated_items\t");
	}

	inline void print_values() {
		num_blocks_created.print("%d\t");
		num_global_blocks.print("%d\t");
		num_inspected_global_items.print("%d\t");
		num_spied_tasks.print("%d\t");
		num_spied_global_tasks.print("%d\t");
		num_merges.print("%d\t");
		num_allocated_items.print("%d\t");
	}

	BasicPerformanceCounter<Pheet, task_storage_count_blocks_created> num_blocks_created;
	BasicPerformanceCounter<Pheet, task_storage_count_global_blocks> num_global_blocks;
	BasicPerformanceCounter<Pheet, task_storage_count_inspected_global_items> num_inspected_global_items;
	BasicPerformanceCounter<Pheet, task_storage_count_spied_tasks> num_spied_tasks;
	BasicPerformanceCounter<Pheet, task_storage_count_spied_global_tasks> num_spied_global_tasks;
	BasicPerformanceCounter<Pheet, task_storage_count_merges> num_merges;
	BasicPerformanceCounter<Pheet, task_storage_count_allocated_items> num_allocated_items;

};

} /* namespace pheet */
#endif /* STRATEGY2BASETASKSTORAGEPERFORMANCECOUNTERS_H_ */
