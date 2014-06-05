/*
 * StrategyBBGraphBipartitioningUpperLowerBoundStrategy.h
 *
 *  Created on: Jan 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPBBGRAPHBIPARTITIONINGUPPERLOWERBOUNDSTRATEGY_H_
#define PPOPPBBGRAPHBIPARTITIONINGUPPERLOWERBOUNDSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/UserDefinedPriority/UserDefinedPriority.h>

namespace pheet {

template <class Pheet, class SubProblem>
  class PPoPPBBGraphBipartitioningUpperLowerBoundStrategy : public Pheet::Environment::BaseStrategy {
public:
    typedef PPoPPBBGraphBipartitioningUpperLowerBoundStrategy<Pheet,SubProblem> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;
    PPoPPBBGraphBipartitioningUpperLowerBoundStrategy(SubProblem* sub_problem);
/*
    PPoPPBBGraphBipartitioningUpperLowerBoundStrategy(Self const& other);
    PPoPPBBGraphBipartitioningUpperLowerBoundStrategy(Self && other);
    */
    ~PPoPPBBGraphBipartitioningUpperLowerBoundStrategy();

    inline bool prioritize(Self& other);
    
    //	UserDefinedPriority<Pheet> operator()(SubProblem* sub_problem, size_t* upper_bound);
    
    static void print_name();
  private:
    size_t upper_bound;
    size_t lower_bound;
  };

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningUpperLowerBoundStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningUpperLowerBoundStrategy(SubProblem* sub_problem):
upper_bound(sub_problem->get_upper_bound()),lower_bound(sub_problem->get_lower_bound()) {
	size_t ub = sub_problem->get_global_upper_bound();;
	size_t w = (ub)/(sub_problem->get_lower_bound() + 1);
	size_t w2 = std::min((size_t)28, sub_problem->size - sub_problem->sets[0].count() - sub_problem->sets[1].count());
	this->set_transitive_weight((size_t)1 << (std::min(w, w2) + 2));
}

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningUpperLowerBoundStrategy<Pheet, SubProblem>::~PPoPPBBGraphBipartitioningUpperLowerBoundStrategy() {

}


template <class Pheet, class SubProblem>
inline bool PPoPPBBGraphBipartitioningUpperLowerBoundStrategy<Pheet, SubProblem>::prioritize(Self& other)
{
	if(this->get_place() == other.get_place() && this->get_place() == Pheet::get_place()) {
		return upper_bound < other.upper_bound;
	}
	return lower_bound < other.lower_bound;
}

template <class Pheet, class SubProblem>
inline void PPoPPBBGraphBipartitioningUpperLowerBoundStrategy<Pheet, SubProblem>::print_name() {
	std::cout << "UpperLowerBoundStrategy";
}

}
#endif /* PPOPPBBGRAPHBIPARTITIONINGUPPERLOWERBOUNDSTRATEGY_H_ */
