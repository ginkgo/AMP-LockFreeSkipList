/*
 * BBGraphBipartitioningPerformanceCounters.h
 *
 *  Created on: Jan 12, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BBGRAPHBIPARTITIONINGPERFORMANCECOUNTERS_H_
#define BBGRAPHBIPARTITIONINGPERFORMANCECOUNTERS_H_

#include <pheet/pheet.h>

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include "BBGraphBipartitioningSubproblemPerformanceCounters.h"

namespace pheet {

template <class Pheet>
class BBGraphBipartitioningPerformanceCounters {
public:
	typedef BBGraphBipartitioningPerformanceCounters<Pheet> Self;

	BBGraphBipartitioningPerformanceCounters();
	BBGraphBipartitioningPerformanceCounters(Self& other);
	~BBGraphBipartitioningPerformanceCounters();

	static void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, graph_bipartitioning_test_count_irrelevant_tasks> num_irrelevant_tasks;
	BBGraphBipartitioningSubproblemPerformanceCounters<Pheet> subproblem_pc;
};

template <class Pheet>
inline BBGraphBipartitioningPerformanceCounters<Pheet>::BBGraphBipartitioningPerformanceCounters()
{

}

template <class Pheet>
inline BBGraphBipartitioningPerformanceCounters<Pheet>::BBGraphBipartitioningPerformanceCounters(Self& other)
:num_irrelevant_tasks(other.num_irrelevant_tasks),
 subproblem_pc(other.subproblem_pc)
{

}

template <class Pheet>
inline BBGraphBipartitioningPerformanceCounters<Pheet>::~BBGraphBipartitioningPerformanceCounters() {

}

template <class Pheet>
inline void BBGraphBipartitioningPerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, graph_bipartitioning_test_count_irrelevant_tasks>::print_header("num_irrelevant_tasks\t");

	BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>::print_headers();
}

template <class Pheet>
inline void BBGraphBipartitioningPerformanceCounters<Pheet>::print_values() {
	num_irrelevant_tasks.print("%d\t");

	subproblem_pc.print_values();
}

}

#endif /* BBGRAPHBIPARTITIONINGPERFORMANCECOUNTERS_H_ */
