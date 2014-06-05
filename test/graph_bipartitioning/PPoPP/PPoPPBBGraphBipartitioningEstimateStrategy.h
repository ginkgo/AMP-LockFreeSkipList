/*
 * StrategyBBGraphBipartitioningEstimateStrategy.h
 *
 *  Created on: Jan 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPBBGRAPHBIPARTITIONINGESTIMATESTRATEGY_H_
#define PPOPPBBGRAPHBIPARTITIONINGESTIMATESTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/UserDefinedPriority/UserDefinedPriority.h>

namespace pheet {

template <class Pheet, class SubProblem>
  class PPoPPBBGraphBipartitioningEstimateStrategy : public Pheet::Environment::BaseStrategy {
public:
    typedef PPoPPBBGraphBipartitioningEstimateStrategy<Pheet,SubProblem> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

    PPoPPBBGraphBipartitioningEstimateStrategy(SubProblem* sub_problem);
//    PPoPPBBGraphBipartitioningEstimateStrategy(Self const& other);
//    PPoPPBBGraphBipartitioningEstimateStrategy(Self && other);


	~PPoPPBBGraphBipartitioningEstimateStrategy();



	//	UserDefinedPriority<Pheet> operator()(SubProblem* sub_problem, size_t* estimate);

	inline bool prioritize(Self& other);
/*
	inline bool forbid_call_conversion() const {
		return !last;
	}*/

	inline bool forbid_call_conversion() const {
		return false;
	}

	inline void rebase() {
		this->reset();
	}

	static void print_name();
private:
    size_t estimate;
    size_t uncertainty;
};

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningEstimateStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningEstimateStrategy(SubProblem* sub_problem)
:estimate(sub_problem->get_estimate()),
 uncertainty(sub_problem->get_estimate() - sub_problem->get_lower_bound()) {
//	this->set_k(1024);
//	size_t depth = sub_problem->sets[0].count() - sub_problem->sets[1].count();
	size_t ub = sub_problem->get_global_upper_bound();;
	size_t lb = sub_problem->get_lower_bound();
	if(lb >= ub) {
		this->set_transitive_weight(1);
	}
	else {
		size_t open = ub - lb;
		size_t w = open / (1+(ub/sub_problem->size));
		this->set_transitive_weight((size_t)1 << (std::min(w>>1, (size_t)28)));
	}
/*	if(ub)
	size_t w = 2 + sub_problem->size - (sub_problem->get_lower_bound())/((ub/sub_problem->size)+1);
	size_t w =
//	size_t w = (ub)/(sub_problem->get_lower_bound());
	size_t w2 = std::min((size_t)28, sub_problem->size - sub_problem->sets[0].count() - sub_problem->sets[1].count());
	this->set_transitive_weight((size_t)1 << (std::min(w, w2)));*/
}
/*
template <class Pheet, class SubProblem>
  inline PPoPPBBGraphBipartitioningEstimateStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningEstimateStrategy(Self const& other):BaseStrategy(other),sub_problem(other.sub_problem),estimate(other.estimate)
{

}

 template <class Pheet, class SubProblem>
   inline PPoPPBBGraphBipartitioningEstimateStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningEstimateStrategy(Self&& other):BaseStrategy(other),sub_problem(other.sub_problem),estimate(other.estimate)
   {

   }
*/

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningEstimateStrategy<Pheet, SubProblem>::~PPoPPBBGraphBipartitioningEstimateStrategy() {

}

template <class Pheet, class SubProblem>
inline bool PPoPPBBGraphBipartitioningEstimateStrategy<Pheet, SubProblem>::prioritize(Self& other)
{
	// Since we rebase all tasks should be from the same place
	pheet_assert(this->get_place() == other.get_place());
	if(this->get_place() == Pheet::get_place()) {
		return estimate < other.estimate;
	}
	return uncertainty > other.uncertainty;
}

/*
template <class Pheet, class SubProblem>
UserDefinedPriority<Pheet> PPoPPBBGraphBipartitioningEstimateStrategy<Pheet, SubProblem>::operator()(SubProblem* sub_problem, size_t* estimate) {
	size_t lb = sub_problem->get_lower_bound();
	size_t ub = sub_problem->get_estimate();

	pheet_assert(ub >= lb);

	// pop task with lowest upper bound (often depth first, but not in extreme cases
	// (upper bound is "best guaranteed upper bound for this subtree" doesn't mean
	// that all branches will be below it!)
	prio_t prio_pop = std::numeric_limits< prio_t >::max() - ub;

	// steal task with highest range of uncertainty
	prio_t prio_steal = ub - lb;

	return UserDefinedPriority<Pheet>(prio_pop, prio_steal);
}
*/


template <class Pheet, class SubProblem>
inline void PPoPPBBGraphBipartitioningEstimateStrategy<Pheet, SubProblem>::print_name() {
	std::cout << "EstimateStrategy";
}

}

#endif /* STRATEGYBBGRAPHBIPARTITIONINGESTIMATESTRATEGY_H_ */
