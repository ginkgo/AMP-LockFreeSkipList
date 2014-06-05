/*
 * BBGraphBipartitioningSubproblemPerformanceCounters.h
 *
 *  Created on: Jan 12, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BBGRAPHBIPARTITIONINGSUBPROBLEMPERFORMANCECOUNTERS_H_
#define BBGRAPHBIPARTITIONINGSUBPROBLEMPERFORMANCECOUNTERS_H_

#include "pheet/pheet.h"

#include "pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "pheet/primitives/PerformanceCounter/Time/TimePerformanceCounter.h"
#include "pheet/primitives/PerformanceCounter/Time/LastTimePerformanceCounter.h"
#include "pheet/primitives/PerformanceCounter/Events/EventsList.h"
#include <vector>


namespace pheet {

template <class Pheet>
class BBGraphBipartitioningSubproblemPerformanceCounters {
public:
	BBGraphBipartitioningSubproblemPerformanceCounters();
	BBGraphBipartitioningSubproblemPerformanceCounters(BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>& other);
	~BBGraphBipartitioningSubproblemPerformanceCounters();

	static void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, graph_bipartitioning_test_count_upper_bound_changes> num_upper_bound_changes;
	BasicPerformanceCounter<Pheet, graph_bipartitioning_test_count_allocated_subproblems> num_allocated_subproblems;

	TimePerformanceCounter<Pheet, graph_bipartitioning_test_measure_memory_allocation_time> memory_allocation_time;
	LastTimePerformanceCounter<Pheet, graph_bipartitioning_test_measure_last_update_time> last_update_time;
	EventsList<Pheet, size_t, false> events;
};


template <class Pheet>
inline BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>::BBGraphBipartitioningSubproblemPerformanceCounters()
{

}

template <class Pheet>
inline BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>::BBGraphBipartitioningSubproblemPerformanceCounters(BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>& other)
:num_upper_bound_changes(other.num_upper_bound_changes),
 num_allocated_subproblems(other.num_allocated_subproblems),
 memory_allocation_time(other.memory_allocation_time),
 last_update_time(other.last_update_time),
 events(other.events)
{

}

template <class Pheet>
inline BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>::~BBGraphBipartitioningSubproblemPerformanceCounters() {

}

template <class Pheet>
inline void BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, graph_bipartitioning_test_count_upper_bound_changes>::print_header("num_upper_bound_changes\t");
	BasicPerformanceCounter<Pheet, graph_bipartitioning_test_count_allocated_subproblems>::print_header("num_allocated_subproblems\t");
	TimePerformanceCounter<Pheet, graph_bipartitioning_test_measure_memory_allocation_time>::print_header("memory_allocation_time\t");
	LastTimePerformanceCounter<Pheet, graph_bipartitioning_test_measure_last_update_time>::print_header("last_update_time\t");

}

template <class Pheet>
inline void BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>::print_values() {
	num_upper_bound_changes.print("%d\t");
	num_allocated_subproblems.print("%d\t");
	memory_allocation_time.print("%f\t");
	last_update_time.print("%f\t");

	events.print();
}

}

#endif /* BBGRAPHBIPARTITIONINGSUBPROBLEMPERFORMANCECOUNTERS_H_ */
