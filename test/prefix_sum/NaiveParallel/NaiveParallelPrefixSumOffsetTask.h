/*
 * NaiveParallelPrefixSumOffsetTask.h
 *
 *  Created on: Jun 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef NAIVEPARALLELPREFIXSUMOFFSETTASK_H_
#define NAIVEPARALLELPREFIXSUMOFFSETTASK_H_

#include <pheet/pheet.h>

namespace pheet {

template <class Pheet, size_t Cutoff>
class NaiveParallelPrefixSumOffsetTask : public Pheet::Task {
public:
	typedef NaiveParallelPrefixSumOffsetTask<Pheet, Cutoff> Self;

	NaiveParallelPrefixSumOffsetTask(unsigned int v, unsigned int* data, size_t length)
	:v(v), data(data), length(length) {}
	virtual ~NaiveParallelPrefixSumOffsetTask() {}

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
#endif /* NAIVEPARALLELPREFIXSUMOFFSETTASK_H_ */
