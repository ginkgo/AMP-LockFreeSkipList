/*
 * StrategyBBGraphBipartitioningDepthFirstStrategy.h
 *
 *  Created on: Dec 21, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYBBGRAPHBIPARTITIONINGDEPTHFIRSTSTRATEGY_H_
#define STRATEGYBBGRAPHBIPARTITIONINGDEPTHFIRSTSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/UserDefinedPriority/UserDefinedPriority.h>

namespace pheet {

template <class Pheet, class SubProblem>
class StrategyBBGraphBipartitioningDepthFirstStrategy {
public:
	StrategyBBGraphBipartitioningDepthFirstStrategy();
	~StrategyBBGraphBipartitioningDepthFirstStrategy();

	UserDefinedPriority<Pheet> operator()(SubProblem* sub_problem);

	static void print_name();
};

template <class Pheet, class SubProblem>
inline StrategyBBGraphBipartitioningDepthFirstStrategy<Pheet, SubProblem>::StrategyBBGraphBipartitioningDepthFirstStrategy() {

}

template <class Pheet, class SubProblem>
inline StrategyBBGraphBipartitioningDepthFirstStrategy<Pheet, SubProblem>::~StrategyBBGraphBipartitioningDepthFirstStrategy() {

}

template <class Pheet, class SubProblem>
UserDefinedPriority<Pheet> StrategyBBGraphBipartitioningDepthFirstStrategy<Pheet, SubProblem>::operator()(SubProblem* sub_problem) {
	// Depth: assumption: good for pop, bad for steal
	size_t depth = sub_problem->sets[0].count() + sub_problem->sets[1].count();

	prio_t prio_pop = 1 + depth;
	prio_t prio_steal = 1 + (sub_problem->size - depth);

	return UserDefinedPriority<Pheet>(prio_pop, prio_steal);
}

template <class Pheet, class SubProblem>
inline void StrategyBBGraphBipartitioningDepthFirstStrategy<Pheet, SubProblem>::print_name() {
	std::cout << "DepthFirstStrategy";
}


}

#endif /* STRATEGYBBGRAPHBIPARTITIONINGDEPTHFIRSTSTRATEGY_H_ */
