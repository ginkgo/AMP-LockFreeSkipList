/*
 * StrategyBBGraphBipartitioningAutoStrategy.h
 *
 *  Created on: Dec 5, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYBBGRAPHBIPARTITIONINGAUTOSTRATEGY_H_
#define STRATEGYBBGRAPHBIPARTITIONINGAUTOSTRATEGY_H_

#include <pheet/pheet.h>
#include "pheet/sched/strategies/LifoFifo/LifoFifoStrategy.h"

namespace pheet {

template <class Pheet, template <class P> class Strategy, class SubProblem>
class StrategyBBGraphBipartitioningAutoStrategyImpl {
public:
	template <template <class P> class NewStrat>
	using WithStrategy = StrategyBBGraphBipartitioningAutoStrategyImpl<Pheet, NewStrat, SubProblem>;

	StrategyBBGraphBipartitioningAutoStrategyImpl();
	~StrategyBBGraphBipartitioningAutoStrategyImpl();

	Strategy<Pheet> operator()(SubProblem* sub_problem);

	static void print_name();
};

template <class Pheet, template <class P> class Strategy, class SubProblem>
inline StrategyBBGraphBipartitioningAutoStrategyImpl<Pheet, Strategy, SubProblem>::StrategyBBGraphBipartitioningAutoStrategyImpl() {

}

template <class Pheet, template <class P> class Strategy, class SubProblem>
inline StrategyBBGraphBipartitioningAutoStrategyImpl<Pheet, Strategy, SubProblem>::~StrategyBBGraphBipartitioningAutoStrategyImpl() {

}

template <class Pheet, template <class P> class Strategy, class SubProblem>
inline Strategy<Pheet> StrategyBBGraphBipartitioningAutoStrategyImpl<Pheet, Strategy, SubProblem>::operator()(SubProblem* sub_problem) {
	return Strategy<Pheet>();
}

template <class Pheet, template <class P> class Strategy, class SubProblem>
void StrategyBBGraphBipartitioningAutoStrategyImpl<Pheet, Strategy, SubProblem>::print_name() {
	std::cout << "AutoStrategy<";
	Strategy<Pheet>::print_name();
	std::cout << ">";
}

template <class Pheet, class SubProblem>
using StrategyBBGraphBipartitioningAutoStrategy = StrategyBBGraphBipartitioningAutoStrategyImpl<Pheet, LifoFifoStrategy, SubProblem>;

}

#endif /* STRATEGYBBGRAPHBIPARTITIONINGAUTOSTRATEGY_H_ */
