/*
 * NumaStrategyPrefixSumLocalSumStrategy.h
 *
 *  Created on: 07.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0
 */

#ifndef NUMASTRATEGYPREFIXSUMLOCALSUMSTRATEGY_H_
#define NUMASTRATEGYPREFIXSUMLOCALSUMSTRATEGY_H_

#include <pheet/ds/StrategyTaskStorage/KLSMLocality/KLSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet, size_t BlockSize>
class NumaStrategyPrefixSumLocalSumStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef NumaStrategyPrefixSumLocalSumStrategy<Pheet, BlockSize> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	typedef KLSMLocalityTaskStorage<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	NumaStrategyPrefixSumLocalSumStrategy() {}

	NumaStrategyPrefixSumLocalSumStrategy(procs_t numa_node, size_t block_id, Place* owner)
	: numa_node(numa_node), block_id(block_id), owner(owner) {}

	NumaStrategyPrefixSumLocalSumStrategy(Self& other)
	: BaseStrategy(other), numa_node(other.numa_node), block_id(other.block_id), owner(other.owner) {}

	NumaStrategyPrefixSumLocalSumStrategy(Self&& other)
	: BaseStrategy(other), numa_node(other.numa_node), block_id(other.block_id), owner(other.owner) {}

	~NumaStrategyPrefixSumLocalSumStrategy() {}

	Self& operator=(Self&& other) {
		numa_node = other.numa_node;
		block_id = other.block_id;
		owner = other.owner;
		return *this;
	}


	bool prioritize(Self& other) {
		Place* p = Pheet::get_place();
		procs_t nnid = p->get_numa_node_id();

		bool is_local = (nnid == numa_node);
		bool is_other_local = (nnid == other.numa_node);

		if(is_local != is_other_local) {
			return is_local;
		}

		size_t d1 = p->get_distance(owner);
		size_t d2 = p->get_distance(other.owner);

		if(d1 > d2)
			return false;
		else if(d1 < d2)
			return true;

		return block_id < other.block_id;
	}

	bool can_call(TaskStoragePlace*) {
		return numa_node == Pheet::get_place()->get_numa_node_id();
	}

	bool dead_task() {
		return false;
	}

	size_t get_k() {
		return 16;
	}

private:
	procs_t numa_node;
	size_t block_id;
	Place* owner;
};

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUMLOCALSUMSTRATEGY_H_ */
