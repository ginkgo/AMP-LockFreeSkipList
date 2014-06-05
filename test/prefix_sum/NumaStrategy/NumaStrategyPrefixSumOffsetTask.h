/*
 * NumaStrategyPrefixSumOffsetTask.h
 *
 *  Created on: Mar 07, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef NUMASTRATEGYPREFIXSUMOFFSETTASK_H_
#define NUMASTRATEGYPREFIXSUMOFFSETTASK_H_

#include <atomic>

#include <pheet/pheet.h>
#include "NumaStrategyPrefixSumStrategy.h"

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive>
class NumaStrategyPrefixSumOffsetTask : public Pheet::Task {
public:
	typedef NumaStrategyPrefixSumOffsetTask<Pheet, BlockSize, Inclusive> Self;
	typedef NumaStrategyPrefixSumStrategy<Pheet, BlockSize> Strategy;
	typedef typename Pheet::Place Place;

	NumaStrategyPrefixSumOffsetTask(unsigned int* data, unsigned int* auxiliary_data, procs_t* numa_nodes, size_t blocks, size_t length, size_t block_id, std::atomic<size_t>& sequential, Place* owner)
	:data(data), auxiliary_data(auxiliary_data), numa_nodes(numa_nodes), blocks(blocks), length(length), block_id(block_id), sequential(sequential), owner(owner) {}
	virtual ~NumaStrategyPrefixSumOffsetTask() {}

	virtual void operator()() {
		if(blocks == 1) {
			if(block_id == sequential.load(std::memory_order_acquire)) {
				unsigned int sum = (block_id == 0)?0:auxiliary_data[block_id - 1];
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
				auxiliary_data[block_id] = sum;
				sequential.store(block_id + 1, std::memory_order_release);
			}
			else {
				unsigned int sum = 0;
				for(size_t i = 0; i < length; ++i) {
					sum += data[i];
				}
				auxiliary_data[block_id] = sum;
			}
		}
		else {
			size_t half = blocks >> 1;
			size_t half_l = half * BlockSize;

//			numa_nodes[half] = Pheet::get_place()->get_data_numa_node_id(data);
			Pheet::template
				spawn_s<Self>(Strategy(numa_nodes[half], block_id + half, owner, false),
						data + half_l, auxiliary_data, numa_nodes + half, blocks - half, length - half_l, block_id + half, sequential, owner);
			Pheet::template
				spawn_s<Self>(Strategy(numa_nodes[0], block_id, owner, sequential.load(std::memory_order_relaxed) == block_id),
						data, auxiliary_data, numa_nodes, half, half_l, block_id, sequential, owner);
		}
	}
private:
	unsigned int* data;
	unsigned int* auxiliary_data;
	procs_t* numa_nodes;
	size_t blocks;
	size_t length;
	size_t block_id;
	std::atomic<size_t>& sequential;
	Place* owner;
};

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUMOFFSETTASK_H_ */
