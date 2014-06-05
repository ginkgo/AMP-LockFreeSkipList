/*
 * StrategyBBGraphBipartitioningDynamicStrategy.h
 *
 *  Created on: Jan 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYBBGRAPHBIPARTITIONINGDYNAMICSTRATEGY_H_
#define STRATEGYBBGRAPHBIPARTITIONINGDYNAMICSTRATEGY_H_

#include <pheet/pheet.h>
#include "StrategyBBGraphBipartitioningUpperBoundFifoStrategy.h"

namespace pheet {

template <class Pheet, template <class P, class SP> class Strategy, class SubProblem>
class StrategyBBGraphBipartitioningDynamicStrategyImpl {
public:
	template <template <class P, class SP> class NewStrat>
	using WithStrategy = StrategyBBGraphBipartitioningDynamicStrategyImpl<Pheet, NewStrat, SubProblem>;

//	template <class Pheet, class SubProblem>
//	using T = StrategyBBGraphBipartitioningDynamicStrategyImpl<Pheet, SubProblem>;

	StrategyBBGraphBipartitioningDynamicStrategyImpl();
	~StrategyBBGraphBipartitioningDynamicStrategyImpl();

	Strategy<Pheet, SubProblem> operator()(SubProblem* sub_problem);

	static void print_name();
};

template <class Pheet, template <class P, class SP> class Strategy, class SubProblem>
inline StrategyBBGraphBipartitioningDynamicStrategyImpl<Pheet, Strategy, SubProblem>::StrategyBBGraphBipartitioningDynamicStrategyImpl() {

}

template <class Pheet, template <class P, class SP> class Strategy, class SubProblem>
inline StrategyBBGraphBipartitioningDynamicStrategyImpl<Pheet, Strategy, SubProblem>::~StrategyBBGraphBipartitioningDynamicStrategyImpl() {

}

template <class Pheet, template <class P, class SP> class Strategy, class SubProblem>
inline Strategy<Pheet, SubProblem> StrategyBBGraphBipartitioningDynamicStrategyImpl<Pheet, Strategy, SubProblem>::operator()(SubProblem* sub_problem) {
	return Strategy<Pheet, SubProblem>(sub_problem);
}

template <class Pheet, template <class P, class SP> class Strategy, class SubProblem>
void StrategyBBGraphBipartitioningDynamicStrategyImpl<Pheet, Strategy, SubProblem>::print_name() {
	std::cout << "DynamicStrategy<";
	Strategy<Pheet, SubProblem>::print_name();
	std::cout << ">";
}

template <class Pheet, class SubProblem>
using StrategyBBGraphBipartitioningDynamicStrategy = StrategyBBGraphBipartitioningDynamicStrategyImpl<Pheet, StrategyBBGraphBipartitioningUpperBoundFifoStrategy, SubProblem>;

}

#endif /* STRATEGYBBGRAPHBIPARTITIONINGDYNAMICSTRATEGY_H_ */
