/*
 * ImprovedBBGraphBipartitioning.h
 *
 *  Created on: Dec 2, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BBGRAPHBIPARTITIONING_H_
#define BBGRAPHBIPARTITIONING_H_

#include "../graph_helpers.h"
#include "BBGraphBipartitioningSubproblem.h"
#include "BBGraphBipartitioningLogic.h"
#include "BBGraphBipartitioningPerformanceCounters.h"
#include "BBGraphBipartitioningTask.h"

#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, template <class P, class SP> class Logic, size_t MaxSize = 64>
class BBGraphBipartitioningImpl {
public:
	typedef BBGraphBipartitioningImpl<Pheet, Logic, MaxSize> Self;
	typedef GraphBipartitioningSolution<MaxSize> Solution;
	typedef MaxReducer<Pheet, Solution> SolutionReducer;
	typedef BBGraphBipartitioningPerformanceCounters<Pheet> PerformanceCounters;
	typedef BBGraphBipartitioningSubproblem<Pheet, Logic, MaxSize> SubProblem;
	typedef BBGraphBipartitioningTask<Pheet, Logic, MaxSize> BBTask;

	template <template <class P, class SP> class NewLogic>
		using WithLogic = BBGraphBipartitioningImpl<Pheet, NewLogic, MaxSize>;

	template <size_t ms>
		using WithMaxSize = BBGraphBipartitioningImpl<Pheet, Logic, ms>;

	template <class P>
		using BT = BBGraphBipartitioningImpl<P, Logic, MaxSize>;

	BBGraphBipartitioningImpl(GraphVertex* data, size_t size, Solution& solution, PerformanceCounters& pc);
	~BBGraphBipartitioningImpl();

	void operator()();

	static void print_headers();
	static void print_configuration();

	static char const name[];

private:
	GraphVertex* data;
	size_t size;
	Solution& solution;
	BBGraphBipartitioningPerformanceCounters<Pheet> pc;
};

template <class Pheet, template <class P, class SP> class Logic, size_t MaxSize>
char const BBGraphBipartitioningImpl<Pheet, Logic, MaxSize>::name[] = "BBGraphBipartitioning";

template <class Pheet, template <class P, class SP> class Logic, size_t MaxSize>
BBGraphBipartitioningImpl<Pheet, Logic, MaxSize>::BBGraphBipartitioningImpl(GraphVertex* data, size_t size, Solution& solution, PerformanceCounters& pc)
: data(data), size(size), solution(solution), pc(pc) {

}

template <class Pheet, template <class P, class SP> class Logic, size_t MaxSize>
BBGraphBipartitioningImpl<Pheet, Logic, MaxSize>::~BBGraphBipartitioningImpl() {

}

template <class Pheet, template <class P, class SP> class Logic, size_t MaxSize>
void BBGraphBipartitioningImpl<Pheet, Logic, MaxSize>::operator()() {
	SolutionReducer best;

	std::atomic<size_t> ub(std::numeric_limits< size_t >::max());

	size_t k = size >> 1;
	SubProblem* prob = new SubProblem(data, size, k, &ub);
	Pheet::template
		finish<BBTask>(prob, best, pc);

	solution = best.get_max();
	pheet_assert(solution.weight == ub.load(std::memory_order_relaxed));
	pheet_assert(ub.load(std::memory_order_relaxed) != std::numeric_limits< size_t >::max());
	pheet_assert(solution.weight != std::numeric_limits< size_t >::max());
	pheet_assert(solution.sets[0].count() == k);
	pheet_assert(solution.sets[1].count() == size - k);
}

template <class Pheet, template <class P, class SP> class Logic, size_t MaxSize>
void BBGraphBipartitioningImpl<Pheet, Logic, MaxSize>::print_configuration() {
	Logic<Pheet, SubProblem>::print_name();
	std::cout << "\t";
}

template <class Pheet, template <class P, class SP> class Logic, size_t MaxSize>
void BBGraphBipartitioningImpl<Pheet, Logic, MaxSize>::print_headers() {
	std::cout << "logic\t";
}

template <class Pheet = Pheet>
using BBGraphBipartitioning = BBGraphBipartitioningImpl<Pheet, BBGraphBipartitioningLogic, 64>;


}

#endif /* BBGRAPHBIPARTITIONING_H_ */
