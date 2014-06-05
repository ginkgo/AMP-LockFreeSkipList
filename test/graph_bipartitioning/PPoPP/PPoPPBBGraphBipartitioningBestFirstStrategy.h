/*
 * StrategyBBGraphBipartitioningBestFirstStrategy.h
 *
 *  Created on: Dec 5, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPBBGRAPHBIPARTITIONINGBESTFIRSTSTRATEGY_H_
#define PPOPPBBGRAPHBIPARTITIONINGBESTFIRSTSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/UserDefinedPriority/UserDefinedPriority.h>

namespace pheet {

template <class Pheet, class SubProblem>
class PPoPPBBGraphBipartitioningBestFirstStrategy : public Pheet::Environment::BaseStrategy {
public:
    typedef PPoPPBBGraphBipartitioningBestFirstStrategy<Pheet,SubProblem> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

    PPoPPBBGraphBipartitioningBestFirstStrategy(SubProblem* sub_problem);
	PPoPPBBGraphBipartitioningBestFirstStrategy(Self const& other);
	PPoPPBBGraphBipartitioningBestFirstStrategy(Self && other);

	~PPoPPBBGraphBipartitioningBestFirstStrategy();

	inline bool prioritize(Self& other);

	inline void rebase() {
		this->reset();
	}

	//	UserDefinedPriority<Pheet> operator()(SubProblem* sub_problem, size_t* upper_bound);

	static void print_name();
private:
	size_t depth;
	size_t lower_bound;
	size_t diff;
};

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningBestFirstStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningBestFirstStrategy(SubProblem* sub_problem)
:depth(std::max(sub_problem->sets[0].count(), sub_problem->sets[1].count())),
 lower_bound(sub_problem->get_lower_bound()),
	// Difference to upper bound - larger is better
	// One maybe good, maybe bad decision: Always use the value relative to the current upper bound
	// Why: gives older elements a slight advantage. They will be rechecked and maybe dropped altogether
	// Also problematic if upper bound is near the limits of size_t as we may have an overflow (shouldn't be a problem in our case)
 diff(sub_problem->get_global_upper_bound() - lower_bound) {
	size_t ub = sub_problem->get_global_upper_bound();
	size_t w = (ub)/(sub_problem->get_lower_bound());
	size_t w2 = std::min((size_t)28, sub_problem->size - sub_problem->sets[0].count() - sub_problem->sets[1].count());
	this->set_transitive_weight((size_t)1 << (std::min(w, w2) + 2));
}

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningBestFirstStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningBestFirstStrategy(Self const& other)
:BaseStrategy(other),
 depth(other.depth),
 lower_bound(other.lower_bound),
 diff(other.diff)
{

}

template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningBestFirstStrategy<Pheet, SubProblem>::PPoPPBBGraphBipartitioningBestFirstStrategy(Self&& other)
:BaseStrategy(other),
 depth(other.depth),
 lower_bound(other.lower_bound),
 diff(other.diff)
{

}


template <class Pheet, class SubProblem>
inline PPoPPBBGraphBipartitioningBestFirstStrategy<Pheet, SubProblem>::~PPoPPBBGraphBipartitioningBestFirstStrategy() {

}

template <class Pheet, class SubProblem>
inline bool PPoPPBBGraphBipartitioningBestFirstStrategy<Pheet, SubProblem>::prioritize(Self& other)
{
	if(this->get_place() == other.get_place() && this->get_place() == Pheet::get_place()) {
		if(depth == other.depth) {
			return diff > other.diff;
		}
		return depth > other.depth;
	}
	return lower_bound * depth < other.lower_bound * other.depth;
}

/*
template <class Pheet, class SubProblem>
UserDefinedPriority<Pheet> PPoPPBBGraphBipartitioningBestFirstStrategy<Pheet, SubProblem>::operator()(SubProblem* sub_problem, size_t* upper_bound) {
	size_t ubc = *upper_bound;

	// Make sure we don't overflow
	size_t limit = (std::numeric_limits< prio_t >::max() - 1) / sub_problem->size;
	if(ubc > limit) {
		// No upper bound available yet
		ubc = limit;
	}

	size_t lb = sub_problem->get_lower_bound();
	if(ubc < lb) {
		// Prioritize this task, as it will immediatly terminate anyway
		// Do not allow it to be stolen (not worth it)
		return UserDefinedPriority<Pheet>(std::numeric_limits< prio_t >::max(), 0);
	}

	// Depth: assumption: good for pop, bad for steal
	size_t depth = std::max(sub_problem->sets[0].count(), sub_problem->sets[1].count());

	// Remaining elements in sets. Presumably lb is more realistic in balanced sets?
	// (ignored for now)
//	size_t set1_remaining = k - set1_size;
//	size_t set2_remaining = (size - k) - set2_size;

	// Difference to upper bound - larger is better
	// One maybe good, maybe bad decision: Always use the value relative to the current upper bound
	// Why: gives older elements a slight advantage. They will be rechecked and maybe dropped altogether
	// Also problematic if upper bound is near the limits of size_t as we may have an overflow (shouldn't be a problem in our case)
	size_t bound_diff = ubc - lb;


	prio_t prio_pop = 1 + depth * bound_diff;
	// use absolute lower-bound for stealing. we don't want to steal something that's probably bad
	prio_t prio_steal = std::numeric_limits< prio_t >::max() - depth * lb;

	return UserDefinedPriority<Pheet>(prio_pop, prio_steal);
}
*/

template <class Pheet, class SubProblem>
inline void PPoPPBBGraphBipartitioningBestFirstStrategy<Pheet, SubProblem>::print_name() {
	std::cout << "BestFirstStrategy";
}

}

#endif /* PPOPPBBGRAPHBIPARTITIONINGBESTFIRSTSTRATEGY_H_ */
