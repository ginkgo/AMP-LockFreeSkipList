/*
 * CentralKStrategyTaskStoragePerformanceCounters.h
 *
 *  Created on: 03.10.2013
 *      Author: mwimmer, mpoeter
 */

#ifndef CENTRALKSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_CPP11_H_
#define CENTRALKSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_CPP11_H_

#include "CentralKStrategyTaskStorageDataBlockPerformanceCounters.h"

namespace pheet { namespace cpp11 {

template <class Pheet, class StrategyHeapPerformanceCounters>
class CentralKStrategyTaskStoragePerformanceCounters {
public:
	typedef CentralKStrategyTaskStoragePerformanceCounters<Pheet, StrategyHeapPerformanceCounters> Self;
	typedef CentralKStrategyTaskStorageDataBlockPerformanceCounters<Pheet> DataBlockPerformanceCounters;

	inline CentralKStrategyTaskStoragePerformanceCounters() {}
	inline CentralKStrategyTaskStoragePerformanceCounters(Self& other)
	:num_blocks_created(other.num_blocks_created),
	 num_unsuccessful_takes(other.num_unsuccessful_takes),
	 num_successful_takes(other.num_successful_takes),
	 num_taken_heap_items(other.num_taken_heap_items),
	 data_block_performance_counters(other.data_block_performance_counters),
	 strategy_heap_performance_counters(other.strategy_heap_performance_counters){}

	inline ~CentralKStrategyTaskStoragePerformanceCounters() {}

	inline static void print_headers() {
		BasicPerformanceCounter<Pheet, task_storage_count_blocks_created>::print_header("num_blocks_created\t");
		BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes>::print_header("num_unsuccessful_takes\t");
		BasicPerformanceCounter<Pheet, task_storage_count_successful_takes>::print_header("num_successful_takes\t");
		BasicPerformanceCounter<Pheet, task_storage_count_taken_heap_items>::print_header("num_taken_heap_items\t");

		DataBlockPerformanceCounters::print_headers();
		StrategyHeapPerformanceCounters::print_headers();
	}

	inline void print_values() {
		num_blocks_created.print("%d\t");
		num_unsuccessful_takes.print("%d\t");
		num_successful_takes.print("%d\t");
		num_taken_heap_items.print("%d\t");

		data_block_performance_counters.print_values();
		strategy_heap_performance_counters.print_values();
	}

	BasicPerformanceCounter<Pheet, task_storage_count_blocks_created> num_blocks_created;
	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes> num_unsuccessful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_successful_takes> num_successful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_taken_heap_items> num_taken_heap_items;

	DataBlockPerformanceCounters data_block_performance_counters;
	StrategyHeapPerformanceCounters strategy_heap_performance_counters;
};

} // namespace cpp11
} // namespace pheet

#endif /* CENTRALKSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_CPP11_H_ */
