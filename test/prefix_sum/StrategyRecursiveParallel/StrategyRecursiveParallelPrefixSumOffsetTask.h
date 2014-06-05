/*
 * StrategyRecursiveParallelPrefixSumOffsetTask.h
 *
 *  Created on: Jun 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGYRECURSIVEPARALLELPREFIXSUMOFFSETTASK_H_
#define STRATEGYRECURSIVEPARALLELPREFIXSUMOFFSETTASK_H_

#include <pheet/pheet.h>
#include "StrategyRecursiveParallelPrefixSumStrategy.h"

namespace pheet {

template <class Pheet, size_t BlockSize>
class StrategyRecursiveParallelPrefixSumOffsetTask : public Pheet::Task {
public:
	typedef StrategyRecursiveParallelPrefixSumOffsetTask<Pheet, BlockSize> Self;
	typedef StrategyRecursiveParallelPrefixSumStrategy<Pheet> Strategy;

	StrategyRecursiveParallelPrefixSumOffsetTask(unsigned int* data, size_t length, size_t step)
	:data(data), length(length), step(step) {}
	virtual ~StrategyRecursiveParallelPrefixSumOffsetTask() {}

	virtual void operator()() {
		if(length <= BlockSize) {
			if(length == BlockSize)
				--length;
			unsigned int v = *(data - step);
			for(size_t i = 0; i < length; ++i) {
				data[i*step] += v;
			}
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;
			size_t half = BlockSize * (num_blocks >> 1);
			Pheet::template
				spawn_s<Self>(
						Strategy(data, half, false, false),
						data, half, step);
			Pheet::template
				spawn_s<Self>(
						Strategy(data + half*step, length - half, false, false),
						data + half*step, length - half, step);
		}
	}
private:
	unsigned int* data;
	size_t length;
	size_t step;
};

} /* namespace pheet */
#endif /* STRATEGYRECURSIVEPARALLELPREFIXSUMOFFSETTASK_H_ */
