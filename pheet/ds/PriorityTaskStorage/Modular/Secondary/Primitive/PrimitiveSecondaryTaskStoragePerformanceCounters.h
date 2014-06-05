/*
 * PrimitiveSecondaryTaskStoragePerformanceCounters.h
 *
 *  Created on: Nov 8, 2011
 *      Author: Martin Wimmer
 */

#ifndef PRIMITIVESECONDARYTASKSTORAGEPERFORMANCECOUNTERS_H_
#define PRIMITIVESECONDARYTASKSTORAGEPERFORMANCECOUNTERS_H_

#include "../../../../../settings.h"

namespace pheet {

template <class Scheduler>
class PrimitiveSecondaryTaskStoragePerformanceCounters {
public:
	PrimitiveSecondaryTaskStoragePerformanceCounters();
	PrimitiveSecondaryTaskStoragePerformanceCounters(PrimitiveSecondaryTaskStoragePerformanceCounters& other);
	~PrimitiveSecondaryTaskStoragePerformanceCounters();

	void print_headers();
	void print_values();

	BasicPerformanceCounter<Scheduler, task_storage_count_steals> num_stolen;
	BasicPerformanceCounter<Scheduler, task_storage_count_unsuccessful_steals> num_unsuccessful_steals;
	BasicPerformanceCounter<Scheduler, task_storage_count_successful_steals> num_successful_steals;
	BasicPerformanceCounter<Scheduler, task_storage_count_size_steal> total_size_steal;
	TimePerformanceCounter<Scheduler, task_storage_measure_steal_time> steal_time;
};

template <class Scheduler>
inline PrimitiveSecondaryTaskStoragePerformanceCounters<Scheduler>::PrimitiveSecondaryTaskStoragePerformanceCounters() {

}

template <class Scheduler>
inline PrimitiveSecondaryTaskStoragePerformanceCounters<Scheduler>::PrimitiveSecondaryTaskStoragePerformanceCounters(PrimitiveSecondaryTaskStoragePerformanceCounters& other)
:num_stolen(other.num_stolen),
 num_unsuccessful_steals(other.num_unsuccessful_steals),
 num_successful_steals(other.num_successful_steals),
 total_size_steal(other.total_size_steal),
 steal_time(other.steal_time)
{

}

template <class Scheduler>
inline PrimitiveSecondaryTaskStoragePerformanceCounters<Scheduler>::~PrimitiveSecondaryTaskStoragePerformanceCounters() {

}

template <class Scheduler>
inline void PrimitiveSecondaryTaskStoragePerformanceCounters<Scheduler>::print_headers() {
	BasicPerformanceCounter<Scheduler, task_storage_count_steals>::print_header("num_stolen\t");
	BasicPerformanceCounter<Scheduler, task_storage_count_unsuccessful_steals>::print_header("num_unsuccessful_steals\t");
	BasicPerformanceCounter<Scheduler, task_storage_count_successful_steals>::print_header("num_successful_steals\t");
	BasicPerformanceCounter<Scheduler, task_storage_count_size_steal>::print_header("total_size_steal\t");
	TimePerformanceCounter<Scheduler, task_storage_measure_steal_time>::print_header("steal_time\t");
}

template <class Scheduler>
inline void PrimitiveSecondaryTaskStoragePerformanceCounters<Scheduler>::print_values() {
	num_stolen.print("%d\t");
	num_unsuccessful_steals.print("%d\t");
	num_successful_steals.print("%d\t");
	total_size_steal.print("%d\t");
	steal_time.print("%f\t");
}

}

#endif /* PRIMITIVESECONDARYTASKSTORAGEPERFORMANCECOUNTERS_H_ */
