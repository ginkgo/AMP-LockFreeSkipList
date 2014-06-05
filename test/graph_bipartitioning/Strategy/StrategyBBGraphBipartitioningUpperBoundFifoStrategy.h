/*
 * StrategyBBGraphBipartitioningUpperBoundFifoStrategy.h
 *
 *  Created on: Jan 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYBBGRAPHBIPARTITIONINGUPPERBOUNDFIFOSTRATEGY_H_
#define STRATEGYBBGRAPHBIPARTITIONINGUPPERBOUNDFIFOSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/BaseStrategy.h>

namespace pheet {

template <class Pheet, class SubProblem>
class StrategyBBGraphBipartitioningUpperBoundFifoStrategy : public BaseStrategy<Pheet> {
public:
	typedef StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem> Self;
	StrategyBBGraphBipartitioningUpperBoundFifoStrategy(SubProblem* sub_problem);
	StrategyBBGraphBipartitioningUpperBoundFifoStrategy(Self const& other);
	StrategyBBGraphBipartitioningUpperBoundFifoStrategy(Self const&& other);
	~StrategyBBGraphBipartitioningUpperBoundFifoStrategy();

	virtual prio_t get_pop_priority(size_t task_id);
	virtual prio_t get_steal_priority(size_t task_id, typename Pheet::Environment::StealerDescriptor& desc);
	virtual BaseStrategy<Pheet>* clone();

	static void print_name();

private:
	prio_t pop_prio;
};

template <class Pheet, class SubProblem>
StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem>::StrategyBBGraphBipartitioningUpperBoundFifoStrategy(SubProblem* sub_problem)
:pop_prio(std::numeric_limits< prio_t >::max() - sub_problem->get_upper_bound())
{

}

template <class Pheet, class SubProblem>
StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem>::StrategyBBGraphBipartitioningUpperBoundFifoStrategy(Self const& other)
:pop_prio(other.pop_prio)
{

}

template <class Pheet, class SubProblem>
StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem>::StrategyBBGraphBipartitioningUpperBoundFifoStrategy(Self const&& other)
:pop_prio(other.pop_prio)
{

}

template <class Pheet, class SubProblem>
StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem>::~StrategyBBGraphBipartitioningUpperBoundFifoStrategy() {

}
template <class Pheet, class SubProblem>
inline prio_t StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem>::get_pop_priority(size_t task_id) {
	return pop_prio;
}

template <class Pheet, class SubProblem>
inline prio_t StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem>::get_steal_priority(size_t task_id, typename Pheet::Environment::StealerDescriptor& desc) {
	return std::numeric_limits< prio_t >::max() - task_id;
}

template <class Pheet, class SubProblem>
inline BaseStrategy<Pheet>* StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem>::clone() {
	return new Self(*this);
}

template <class Pheet, class SubProblem>
inline void StrategyBBGraphBipartitioningUpperBoundFifoStrategy<Pheet, SubProblem>::print_name() {
	std::cout << "UpperBoundFifoStrategy";
}

}

#endif /* STRATEGYBBGRAPHBIPARTITIONINGUPPERBOUNDFIFOSTRATEGY_H_ */
