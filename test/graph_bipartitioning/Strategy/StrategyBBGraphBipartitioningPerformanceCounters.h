/*
 * StrategyBBGraphBipartitioningPerformanceCounters.h
 *
 *  Created on: Jan 12, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYBBGRAPHBIPARTITIONINGPERFORMANCECOUNTERS_H_
#define STRATEGYBBGRAPHBIPARTITIONINGPERFORMANCECOUNTERS_H_

#include "pheet/pheet.h"

#include "pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "../Basic/BBGraphBipartitioningSubproblemPerformanceCounters.h"

namespace pheet {

template <class Pheet>
class StrategyBBGraphBipartitioningPerformanceCounters {
public:
	StrategyBBGraphBipartitioningPerformanceCounters();
	StrategyBBGraphBipartitioningPerformanceCounters(StrategyBBGraphBipartitioningPerformanceCounters& other);
	StrategyBBGraphBipartitioningPerformanceCounters(StrategyBBGraphBipartitioningPerformanceCounters&& other);
	~StrategyBBGraphBipartitioningPerformanceCounters();

	static void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, graph_bipartitioning_test_count_irrelevant_tasks> num_irrelevant_tasks;
	BBGraphBipartitioningSubproblemPerformanceCounters<Pheet> subproblem_pc;
};


template <class Pheet>
inline StrategyBBGraphBipartitioningPerformanceCounters<Pheet>::StrategyBBGraphBipartitioningPerformanceCounters()
{

}

template <class Pheet>
inline StrategyBBGraphBipartitioningPerformanceCounters<Pheet>::StrategyBBGraphBipartitioningPerformanceCounters(StrategyBBGraphBipartitioningPerformanceCounters<Pheet>& other)
:num_irrelevant_tasks(other.num_irrelevant_tasks),
 subproblem_pc(other.subproblem_pc)
{

}

template <class Pheet>
inline StrategyBBGraphBipartitioningPerformanceCounters<Pheet>::StrategyBBGraphBipartitioningPerformanceCounters(StrategyBBGraphBipartitioningPerformanceCounters<Pheet>&& other)
:num_irrelevant_tasks(other.num_irrelevant_tasks),
 subproblem_pc(other.subproblem_pc)
{

}

template <class Pheet>
inline StrategyBBGraphBipartitioningPerformanceCounters<Pheet>::~StrategyBBGraphBipartitioningPerformanceCounters() {

}

template <class Pheet>
inline void StrategyBBGraphBipartitioningPerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, graph_bipartitioning_test_count_irrelevant_tasks>::print_header("num_irrelevant_tasks\t");

	BBGraphBipartitioningSubproblemPerformanceCounters<Pheet>::print_headers();
}

template <class Pheet>
inline void StrategyBBGraphBipartitioningPerformanceCounters<Pheet>::print_values() {
	num_irrelevant_tasks.print("%d\t");

	subproblem_pc.print_values();
}

}

#endif /* STRATEGYBBGRAPHBIPARTITIONINGPERFORMANCECOUNTERS_H_ */
