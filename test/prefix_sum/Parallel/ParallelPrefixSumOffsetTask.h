/*
 * ParallelPrefixSumOffsetTask.h
 *
 *  Created on: Jun 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef PARALLELPREFIXSUMOFFSETTASK_H_
#define PARALLELPREFIXSUMOFFSETTASK_H_

#include <pheet/pheet.h>

namespace pheet {

template <class Pheet, size_t Cutoff>
class ParallelPrefixSumOffsetTask : public Pheet::Task {
public:
	typedef ParallelPrefixSumOffsetTask<Pheet, Cutoff> Self;

	ParallelPrefixSumOffsetTask(unsigned int v, unsigned int* data, size_t length)
	:v(v), data(data), length(length) {}
	virtual ~ParallelPrefixSumOffsetTask() {}

	virtual void operator()() {
		if(length <= Cutoff) {
			for(size_t i = 0; i < length; ++i) {
				data[i] += v;
			}
		}
		else {
			size_t half = length >> 1;
			Pheet::template
				spawn<Self>(v, data, half);
			Pheet::template
				spawn<Self>(v, data + half, length - half);
		}
	}
private:
	unsigned int v;
	unsigned int* data;
	size_t length;
};

} /* namespace pheet */
#endif /* PARALLELPREFIXSUMOFFSETTASK_H_ */
