/*
 * RecursiveParallelPrefixSumOffsetTask.h
 *
 *  Created on: Jun 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUMOFFSETTASK_H_
#define RECURSIVEPARALLELPREFIXSUMOFFSETTASK_H_

#include <pheet/pheet.h>

namespace pheet {

template <class Pheet, size_t BlockSize>
class RecursiveParallelPrefixSumOffsetTask : public Pheet::Task {
public:
	typedef RecursiveParallelPrefixSumOffsetTask<Pheet, BlockSize> Self;

	RecursiveParallelPrefixSumOffsetTask(unsigned int* data, size_t length, size_t step)
	:data(data), length(length), step(step) {}
	virtual ~RecursiveParallelPrefixSumOffsetTask() {}

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
				spawn<Self>(data, half, step);
			Pheet::template
				spawn<Self>(data + half*step, length - half, step);
		}
	}
private:
	unsigned int* data;
	size_t length;
	size_t step;
};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUMOFFSETTASK_H_ */
