/*
 * NumaStrategyPrefixSumStrategy.h
 *
 *  Created on: 07.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0
 */

#ifndef NUMASTRATEGYPREFIXSUMSTRATEGY_H_
#define NUMASTRATEGYPREFIXSUMSTRATEGY_H_

#include <pheet/ds/StrategyTaskStorage/KLSMLocality/KLSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet, size_t BlockSize>
class NumaStrategyPrefixSumStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef NumaStrategyPrefixSumStrategy<Pheet, BlockSize> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	typedef KLSMLocalityTaskStorage<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	NumaStrategyPrefixSumStrategy() {}

	NumaStrategyPrefixSumStrategy(procs_t numa_node, size_t block_id, Place* owner, bool in_order)
	: numa_node(numa_node), block_id(block_id), owner(owner), in_order(in_order) {}

	NumaStrategyPrefixSumStrategy(Self& other)
	: BaseStrategy(other), numa_node(other.numa_node), block_id(other.block_id), owner(other.owner), in_order(in_order) {}

	NumaStrategyPrefixSumStrategy(Self&& other)
	: BaseStrategy(other), numa_node(other.numa_node), block_id(other.block_id), owner(other.owner), in_order(in_order) {}

	~NumaStrategyPrefixSumStrategy() {}

	Self& operator=(Self&& other) {
		numa_node = other.numa_node;
		block_id = other.block_id;
		owner = other.owner;
		in_order = other.in_order;
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
		return 16;
	}

private:
	procs_t numa_node;
	size_t block_id;
	Place* owner;
	bool in_order;
};

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUMSTRATEGY_H_ */
