/*
 * SmartRecursiveParallelPrefixSum2OffsetTask.h
 *
 *  Created on: Mar 07, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SMARTRECURSIVEPARALLELPREFIXSUM2OFFSETTASK_H_
#define SMARTRECURSIVEPARALLELPREFIXSUM2OFFSETTASK_H_

#include <pheet/pheet.h>

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive>
class SmartRecursiveParallelPrefixSum2OffsetTask : public Pheet::Task {
public:
	typedef SmartRecursiveParallelPrefixSum2OffsetTask<Pheet, BlockSize, Inclusive> Self;

	SmartRecursiveParallelPrefixSum2OffsetTask(unsigned int* data, unsigned int* auxiliary_data, size_t blocks, size_t length, size_t block_id, std::atomic<size_t>& sequential)
	:data(data), auxiliary_data(auxiliary_data), blocks(blocks), length(length), block_id(block_id), sequential(sequential) {}
	virtual ~SmartRecursiveParallelPrefixSum2OffsetTask() {}

	virtual void operator()() {
		if(blocks == 1) {
			if(block_id == sequential.load(std::memory_order_relaxed)) {
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

			Pheet::template
				spawn<Self>(data + half_l, auxiliary_data, blocks - half, length - half_l, block_id + half, sequential);

			Pheet::template
				call<Self>(data, auxiliary_data, half, half_l, block_id, sequential);
		}
	}
private:
	unsigned int* data;
	unsigned int* auxiliary_data;
	size_t blocks;
	size_t length;
	size_t block_id;
	std::atomic<size_t>& sequential;
};

} /* namespace pheet */
#endif /* SMARTRECURSIVEPARALLELPREFIXSUM2OFFSETTASK_H_ */
