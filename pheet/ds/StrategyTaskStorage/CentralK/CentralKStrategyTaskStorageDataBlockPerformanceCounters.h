/*
 * CentralKStrategyTaskStorageDataBlockPerformanceCounters.h
 *
 *  Created on: 06.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef CENTRALKSTRATEGYTASKSTORAGEDATABLOCKPERFORMANCECOUNTERS_H_
#define CENTRALKSTRATEGYTASKSTORAGEDATABLOCKPERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet>
class CentralKStrategyTaskStorageDataBlockPerformanceCounters {
public:
	typedef CentralKStrategyTaskStorageDataBlockPerformanceCounters<Pheet> Self;

	inline CentralKStrategyTaskStorageDataBlockPerformanceCounters() {}

	inline CentralKStrategyTaskStorageDataBlockPerformanceCounters(Self& other)
	:num_take_tests(other.num_take_tests),
	 num_put_tests(other.num_put_tests) {}

	inline ~CentralKStrategyTaskStorageDataBlockPerformanceCounters() {}

	inline static void print_headers() {
//		BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes>::print_header("num_unsuccessful_takes\t");
//		BasicPerformanceCounter<Pheet, task_storage_count_successful_takes>::print_header("num_successful_takes\t");
		BasicPerformanceCounter<Pheet, task_storage_count_take_tests>::print_header("num_take_tests\t");
		BasicPerformanceCounter<Pheet, task_storage_count_put_tests>::print_header("num_put_tests\t");
	}

	inline void print_values() {
//		num_unsuccessful_takes.print("%d\t");
//		num_successful_takes.print("%d\t");
		num_take_tests.print("%d\t");
		num_put_tests.print("%d\t");
	}

//	BasicPerformanceCounter<Pheet, task_storage_count_unsuccessful_takes> num_unsuccessful_takes;
//	BasicPerformanceCounter<Pheet, task_storage_count_successful_takes> num_successful_takes;
	BasicPerformanceCounter<Pheet, task_storage_count_take_tests> num_take_tests;
	BasicPerformanceCounter<Pheet, task_storage_count_put_tests> num_put_tests;
};

}

#endif /* CENTRALKSTRATEGYTASKSTORAGEDATABLOCKPERFORMANCECOUNTERS_H_ */
