/*
 * RecursiveParallelPrefixSum2OffsetTask.h
 *
 *  Created on: Mar 07, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUM2OFFSETTASK_H_
#define RECURSIVEPARALLELPREFIXSUM2OFFSETTASK_H_

#include <pheet/pheet.h>

namespace pheet {

template <class Pheet, size_t BlockSize>
class RecursiveParallelPrefixSum2OffsetTask : public Pheet::Task {
public:
	typedef RecursiveParallelPrefixSum2OffsetTask<Pheet, BlockSize> Self;

	RecursiveParallelPrefixSum2OffsetTask(unsigned int* data, unsigned int* auxiliary_data, size_t blocks, size_t length)
	:data(data), auxiliary_data(auxiliary_data), blocks(blocks), length(length) {}
	virtual ~RecursiveParallelPrefixSum2OffsetTask() {}

	virtual void operator()() {
		if(blocks == 1) {
			unsigned int sum = 0;
			for(size_t i = 0; i < length; ++i) {
				sum += data[i];
			}
			auxiliary_data[0] = sum;
		}
		else {
			size_t half = blocks >> 1;
			size_t half_l = half * BlockSize;

			Pheet::template
				spawn<Self>(data + half_l, auxiliary_data + half, blocks - half, length - half_l);

			Pheet::template
				call<Self>(data, auxiliary_data, half, half_l);
		}
	}
private:
	unsigned int* data;
	unsigned int* auxiliary_data;
	size_t blocks;
	size_t length;
};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUM2OFFSETTASK_H_ */
