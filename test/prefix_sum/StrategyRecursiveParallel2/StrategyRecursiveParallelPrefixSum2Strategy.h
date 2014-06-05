/*
 * StrategyRecursiveParallelPrefixSum2Strategy.h
 *
 *  Created on: 07.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0
 */

#ifndef STRATEGYRECURSIVEPARALLELPREFIXSUM2STRATEGY_H_
#define STRATEGYRECURSIVEPARALLELPREFIXSUM2STRATEGY_H_

#include <pheet/ds/StrategyTaskStorage/KLSMLocality/KLSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet>
class StrategyRecursiveParallelPrefixSum2Strategy : public Pheet::Environment::BaseStrategy {
public:
	typedef StrategyRecursiveParallelPrefixSum2Strategy<Pheet> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	typedef KLSMLocalityTaskStorage<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	StrategyRecursiveParallelPrefixSum2Strategy() {}

	StrategyRecursiveParallelPrefixSum2Strategy(size_t block_id, Place* owner, bool in_order)
	: block_id(block_id), owner(owner), in_order(in_order) {}

	StrategyRecursiveParallelPrefixSum2Strategy(Self& other)
	: BaseStrategy(other), block_id(other.block_id), owner(other.owner), in_order(in_order) {}

	StrategyRecursiveParallelPrefixSum2Strategy(Self&& other)
	: BaseStrategy(other), block_id(other.block_id), owner(other.owner), in_order(in_order) {}

	~StrategyRecursiveParallelPrefixSum2Strategy() {}

	Self& operator=(Self&& other) {
		block_id = other.block_id;
		owner = other.owner;
		in_order = other.in_order;
		return *this;
	}


	bool prioritize(Self& other) {
		Place* p = Pheet::get_place();
		if(owner == p) {
			if(other.owner != p) {
				return true;
			}
			return block_id < other.block_id;
		}
		else if(other.owner == p) {
			return false;
		}
		return block_id > other.block_id;
	}

	bool can_call(TaskStoragePlace*) {
		return in_order;
	}

	bool dead_task() {
		return false;
	}

	size_t get_k() {
		return 8;
	}

private:
	size_t block_id;
	Place* owner;
	bool in_order;
};

} /* namespace pheet */
#endif /* STRATEGYRECURSIVEPARALLELPREFIXSUM2STRATEGY_H_ */
