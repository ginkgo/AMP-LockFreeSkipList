/*
 * PPoPPBBGraphBipartitioningLowerBoundStrategy.h
 *
 *  Created on: Dec 21, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPBBGRAPHBIPARTITIONINGLOWERBOUNDSTRATEGY_H_
#define PPOPPBBGRAPHBIPARTITIONINGLOWERBOUNDSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/UserDefinedPriority/UserDefinedPriority.h>

namespace pheet {

template <class Pheet, class SubProblem>
class PPoPPBBGraphBipartitioningLowerBoundStrategy : public Pheet::Environment::BaseStrategy {
public:
    typedef PPoPPBBGraphBipartitioningLowerBoundStrategy<Pheet,SubProblem> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;
    
    PPoPPBBGraphBipartitioningLowerBoundStrategy(SubProblem* sub_problem);
    PPoPPBBGraphBipartitioningLowerBoundStrategy(Self const& other);
    PPoPPBBGraphBipartitioningLowerBoundStrategy(Self && other);

	~PPoPPBBGraphBipartitioningLowerBoundStrategy();

	inline bool prioritize(Self& other);

	//	UserDefinedPriority<Pheet> operator()(SubProblem* sub_problem, size_t* upper_bound);

	static void print_name();
private:
	size_t lower_bound;
};

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningLowerBoundStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningLowerBoundStrategy(SubProblem* sub_problem)
:lower_bound(sub_problem->get_lower_bound()) {
	size_t ub = sub_problem->get_global_upper_bound();;
	size_t w = (ub)/(sub_problem->get_lower_bound() + 1);
	size_t w2 = std::min((size_t)28, sub_problem->size - sub_problem->sets[0].count() - sub_problem->sets[1].count());
	this->set_transitive_weight((size_t)1 << (std::min(w, w2) + 2));
}

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningLowerBoundStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningLowerBoundStrategy(Self const& other)
:BaseStrategy(other),
 lower_bound(other.lower_bound)
{

}

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningLowerBoundStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningLowerBoundStrategy(Self&& other)
:BaseStrategy(other),
 lower_bound(other.lower_bound)
{

}

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningLowerBoundStrategy<Pheet, SubProblem>::~PPoPPBBGraphBipartitioningLowerBoundStrategy() {

}

template <class Pheet, class SubProblem>
inline bool PPoPPBBGraphBipartitioningLowerBoundStrategy<Pheet, SubProblem>::prioritize(Self& other)
{
	return lower_bound < other.lower_bound;
}

/*
template <class Pheet, class SubProblem>
UserDefinedPriority<Pheet> PPoPPBBGraphBipartitioningLowerBoundStrategy<Pheet, SubProblem>::operator()(SubProblem* sub_problem, size_t* upper_bound) {
	size_t lb = sub_problem->get_lower_bound();
	size_t ub = *upper_bound;

//	prio_t prio_pop = 1 + depth * bound_diff;
//	prio_t prio_steal = 1 + (sub_problem->size - depth) * bound_diff;

	return UserDefinedPriority<Pheet>(ub - lb, ub - lb);
	}*/

template <class Pheet, class SubProblem>
inline void PPoPPBBGraphBipartitioningLowerBoundStrategy<Pheet, SubProblem>::print_name() {
	std::cout << "LowerBoundStrategy";
}

}

#endif /* PPOPPBBGRAPHBIPARTITIONINGLOWERBOUNDSTRATEGY_H_ */
