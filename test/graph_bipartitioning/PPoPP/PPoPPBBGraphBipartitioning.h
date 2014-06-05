/*
 * StrategyBBGraphBipartitioning.h
 *
 *  Created on: Dec 5, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPBBGRAPHBIPARTITIONING_H_
#define PPOPPBBGRAPHBIPARTITIONING_H_

#include "../graph_helpers.h"
#include "../Basic/BBGraphBipartitioningSubproblem.h"
#include "../Basic/BBGraphBipartitioningLogic.h"
#include "PPoPPBBGraphBipartitioningEstimateStrategy.h"
#include "PPoPPBBGraphBipartitioningStrategy2.h"
#include "../Strategy/StrategyBBGraphBipartitioningPerformanceCounters.h"
#include "PPoPPBBGraphBipartitioningTask.h"

#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, template <class P, class SP> class Logic, template <class P, class SubProblem> class SchedulingStrategy, size_t MaxSize = 64>
class PPoPPBBGraphBipartitioningImpl {
public:
	typedef PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, MaxSize> Self;
	typedef GraphBipartitioningSolution<MaxSize> Solution;
	typedef MaxReducer<Pheet, Solution> SolutionReducer;
	typedef StrategyBBGraphBipartitioningPerformanceCounters<Pheet> PerformanceCounters;
	typedef BBGraphBipartitioningSubproblem<Pheet, Logic, MaxSize> SubProblem;
	typedef PPoPPBBGraphBipartitioningTask<Pheet, Logic, SchedulingStrategy, MaxSize> BBTask;

	template <template <class P, class SP> class NewLogic>
		using WithLogic = PPoPPBBGraphBipartitioningImpl<Pheet, NewLogic, SchedulingStrategy, MaxSize>;

	template <template <class P, class SP> class NewStrat>
		using WithSchedulingStrategy = PPoPPBBGraphBipartitioningImpl<Pheet, Logic, NewStrat, MaxSize>;

	template <size_t ms>
		using WithMaxSize = PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, ms>;

	template <class P>
		using BT = PPoPPBBGraphBipartitioningImpl<P, Logic, SchedulingStrategy, MaxSize>;


	PPoPPBBGraphBipartitioningImpl(GraphVertex* data, size_t size, Solution& solution, PerformanceCounters& pc);
	~PPoPPBBGraphBipartitioningImpl();

	void operator()();

	static void print_headers();
	static void print_configuration();

	static char const name[];

private:
	GraphVertex* data;
	size_t size;
	Solution& solution;
	PerformanceCounters pc;
};

template <class Pheet, template <class P, class SubProblem> class Logic, template <class P, class SubProblem> class SchedulingStrategy, size_t MaxSize>
char const PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, MaxSize>::name[] = "PPoPPBBGraphBipartitioning";

template <class Pheet, template <class P, class SubProblem> class Logic, template <class P, class SubProblem> class SchedulingStrategy, size_t MaxSize>
PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, MaxSize>::PPoPPBBGraphBipartitioningImpl(GraphVertex* data, size_t size, Solution& solution, PerformanceCounters& pc)
: data(data), size(size), solution(solution), pc(pc) {

}

template <class Pheet, template <class P, class SubProblem> class Logic, template <class P, class SubProblem> class SchedulingStrategy, size_t MaxSize>
PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, MaxSize>::~PPoPPBBGraphBipartitioningImpl() {

}

template <class Pheet, template <class P, class SubProblem> class Logic, template <class P, class SubProblem> class SchedulingStrategy, size_t MaxSize>
void PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, MaxSize>::operator()() {
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
/*
template <class Pheet, template <class P, class SubProblem> class Logic, template <class P, class SubProblem> class SchedulingStrategy, size_t MaxSize>
void PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, MaxSize>::partition(SubProblem* sub_problem, size_t* upper_bound, SolutionReducer& best, PerformanceCounters& pc) {
	if(sub_problem->get_lower_bound() >= *upper_bound) {
		delete sub_problem;
		pc.num_irrelevant_tasks.incr();
		return;
	}
	SchedulingStrategy<Pheet, SubProblem> strategy;

	SubProblem* sub_problem2 =
			sub_problem->split(pc.subproblem_pc);

	if(sub_problem->is_solution()) {
		sub_problem->update_solution(upper_bound, best, pc.subproblem_pc);
		delete sub_problem;
	}
	else if(sub_problem->get_lower_bound() < *upper_bound) {
		Pheet::spawn_prio(strategy(sub_problem, upper_bound),
				Self::partition,
				sub_problem, upper_bound, std::move(SolutionReducer(best)), std::move(PerformanceCounters(pc)));
	}
	else {
		delete sub_problem;
	}

	if(sub_problem2->is_solution()) {
		sub_problem2->update_solution(upper_bound, best, pc.subproblem_pc);
		delete sub_problem2;
	}
	else if(sub_problem2->get_lower_bound() < *upper_bound) {
		Pheet::spawn_prio(strategy(sub_problem2, upper_bound),
				partition,
				sub_problem2, upper_bound, best, pc);
	}
	else {
		delete sub_problem2;
	}
}*/

template <class Pheet, template <class P, class SubProblem> class Logic, template <class P, class SubProblem> class SchedulingStrategy, size_t MaxSize>
void PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, MaxSize>::print_configuration() {
	Logic<Pheet, SubProblem>::print_name();
	std::cout << "\t";
	SchedulingStrategy<Pheet, SubProblem>::print_name();
	std::cout << "\t";
}

template <class Pheet, template <class P, class SubProblem> class Logic, template <class P, class SubProblem> class SchedulingStrategy, size_t MaxSize>
void PPoPPBBGraphBipartitioningImpl<Pheet, Logic, SchedulingStrategy, MaxSize>::print_headers() {
	std::cout << "logic\tstrategy\t";
}

template <class Pheet = Pheet>
using PPoPPBBGraphBipartitioning = PPoPPBBGraphBipartitioningImpl<Pheet, BBGraphBipartitioningLogic, PPoPPBBGraphBipartitioningEstimateStrategy, 64>;

template <class Pheet = Pheet>
using PPoPPBBGraphBipartitioning2 = PPoPPBBGraphBipartitioningImpl<Pheet, BBGraphBipartitioningLogic, PPoPPBBGraphBipartitioningStrategy2, 64>;

/*
template <class Pheet = Pheet>
using OldPPoPPBBGraphBipartitioning = PPoPPBBGraphBipartitioningImpl<Pheet, BBGraphBipartitioningLogic, PPoPPBBGraphBipartitioningDepthFirstBestStrategy, 64>;
*/
}

#endif /* PPOPPBBGRAPHBIPARTITIONING_H_ */
