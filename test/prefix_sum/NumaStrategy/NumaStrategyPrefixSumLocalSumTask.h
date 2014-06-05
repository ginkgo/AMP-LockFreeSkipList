/*
 * NumaStrategyPrefixSumLocalSumTask.h
 *
 *  Created on: 20.03.2014
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef NUMASTRATEGYPREFIXSUMLOCALSUMTASK_H_
#define NUMASTRATEGYPREFIXSUMLOCALSUMTASK_H_

#include "NumaStrategyPrefixSumLocalSumStrategy.h"
#include "NumaStrategyPrefixSumPerformanceCounters.h"

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive>
class NumaStrategyPrefixSumLocalSumTask : public Pheet::Task {
public:
	typedef NumaStrategyPrefixSumLocalSumTask<Pheet, BlockSize, Inclusive> Self;

	typedef NumaStrategyPrefixSumLocalSumStrategy<Pheet, BlockSize> Strategy;
	typedef NumaStrategyPrefixSumPerformanceCounters<Pheet> PerformanceCounters;

	NumaStrategyPrefixSumLocalSumTask(unsigned int* data, unsigned int* auxiliary_data, procs_t* numa_nodes, size_t blocks, size_t length, size_t block_id, PerformanceCounters& pc)
		:data(data), auxiliary_data(auxiliary_data), numa_nodes(numa_nodes), blocks(blocks), length(length), block_id(block_id), pc(pc) {}
	~NumaStrategyPrefixSumLocalSumTask() {}

	virtual void operator()() {
		if(numa_nodes[0] == Pheet::get_place()->get_numa_node_id()) {
			pc.numa_local_blocks.incr();
		}
		else {
			pc.numa_non_local_blocks.incr();
		}
		if(blocks == 1) {
			unsigned int sum = auxiliary_data[0];
			for(size_t i = 0; i < length; ++i) {
				unsigned int tmp = data[i];
				if(Inclusive) {
					sum += tmp;
					data[i] = sum;
				}
				else {
					data[i] = sum;
					sum += tmp;
				}
			}
		}
		else {
			size_t half = blocks >> 1;
			size_t half_l = half * BlockSize;

			auto p = Pheet::get_place();
			Pheet::template
				spawn_s<Self>(Strategy(numa_nodes[half], block_id + half, p),
						data + half_l, auxiliary_data + half, numa_nodes + half, blocks - half, length - half_l, block_id + half, pc);

			Pheet::template
				spawn_s<Self>(Strategy(numa_nodes[0], block_id, p),
						data, auxiliary_data, numa_nodes, half, half_l, block_id, pc);
		}
	}

private:
	unsigned int* data;
	unsigned int* auxiliary_data;
	procs_t* numa_nodes;
	size_t blocks;
	size_t length;
	size_t block_id;
	PerformanceCounters pc;
};

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUMLOCALSUMTASK_H_ */
