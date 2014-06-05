/*
 * StrategyBBGraphBipartitioningStrategy.h
 *
 *  Created on: Mar 14, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPBBGRAPHBIPARTITIONINGSTRATEGY2_H_
#define PPOPPBBGRAPHBIPARTITIONINGSTRATEGY2_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/UserDefinedPriority/UserDefinedPriority.h>

#include <pheet/ds/StrategyTaskStorage/LSMLocality/LSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet, class SubProblem>
  class PPoPPBBGraphBipartitioningStrategy2 : public Pheet::Environment::BaseStrategy {
public:
    typedef PPoPPBBGraphBipartitioningStrategy2<Pheet,SubProblem> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

    typedef LSMLocalityTaskStorage<Pheet, Self> TaskStorage;
    typedef typename TaskStorage::Place TaskStoragePlace;

    typedef typename Pheet::Place Place;

    PPoPPBBGraphBipartitioningStrategy2()
    :place(nullptr) {}

    PPoPPBBGraphBipartitioningStrategy2(SubProblem* sub_problem)
    :place(Pheet::get_place()),
     sub_problem(sub_problem),
     lower_bound(sub_problem->get_lower_bound()),
     estimate(sub_problem->get_estimate()),
     uncertainty(sub_problem->get_estimate() - sub_problem->get_lower_bound()),
//     treat_local(estimate < sub_problem->get_global_upper_bound()),
     upper_bound(sub_problem->upper_bound)
    {}

    PPoPPBBGraphBipartitioningStrategy2(Self& other)
    : BaseStrategy(other),
      place(other.place),
      sub_problem(other.sub_problem),
      lower_bound(other.lower_bound),
      estimate(other.estimate),
      uncertainty(other.uncertainty),
//      treat_local(other.treat_local),
      upper_bound(other.upper_bound) {}

    PPoPPBBGraphBipartitioningStrategy2(Self&& other)
    : BaseStrategy(other),
      place(other.place),
      sub_problem(other.sub_problem),
      lower_bound(other.lower_bound),
      estimate(other.estimate),
      uncertainty(other.uncertainty),
//      treat_local(other.treat_local),
      upper_bound(other.upper_bound) {}


	~PPoPPBBGraphBipartitioningStrategy2() {}

	Self& operator=(Self&& other) {
		place = other.place;
		sub_problem = other.sub_problem;
		lower_bound = other.lower_bound;
		estimate = other.estimate;
		uncertainty = other.uncertainty;
//		treat_local = other.treat_local;
		upper_bound = other.upper_bound;
		return *this;
	}

	inline bool prioritize(Self& other) {
		Place* p = Pheet::get_place();
		if(this->place == p) {
			 if(other.place == p)
				return estimate < other.estimate;
			 else
				 return true;
		}
		else if(other.place == p) {
			return false;
		}
		return uncertainty > other.uncertainty;
	}

	bool dead_task() {
		return lower_bound == upper_bound->load(std::memory_order_relaxed);
	}

	/*
	 * Checks whether spawn can be converted to a function call
	 */
	inline bool can_call(TaskStoragePlace* p) {
		size_t ub = upper_bound->load(std::memory_order_relaxed);
		if(lower_bound >= ub)
			return true;
		size_t diff = ub - lower_bound;
		size_t open = (diff / (1 + (ub/sub_problem->size))) >> 1;

		return open < 28 && (p->size()*p->size() >> open) > 0;
	}

	static void print_name() {
		std::cout << "Strategy2";
	}
private:
	Place* place;
	SubProblem* sub_problem;
	size_t lower_bound;
    size_t estimate;
    size_t uncertainty;
//    bool treat_local;
    std::atomic<size_t>* upper_bound;
};

}

#endif /* STRATEGYBBGRAPHBIPARTITIONINGSTRATEGY2_H_ */
