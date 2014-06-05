/*
 * StrategyPrefixSumOffsetTask.h
 *
 *  Created on: Jun 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGYPREFIXSUMOFFSETTASK_H_
#define STRATEGYPREFIXSUMOFFSETTASK_H_

#include <pheet/pheet.h>
#include "StrategyPrefixSumStrategy.h"

namespace pheet {

template <class Pheet, size_t Cutoff>
class StrategyPrefixSumOffsetTask : public Pheet::Task {
public:
	typedef StrategyPrefixSumOffsetTask<Pheet, Cutoff> Self;
	typedef StrategyPrefixSumStrategy<Pheet> Strategy;

	StrategyPrefixSumOffsetTask(unsigned int v, unsigned int* original_offset, unsigned int* data, size_t length)
	:v(v), original_offset(original_offset), data(data), length(length) {}
	virtual ~StrategyPrefixSumOffsetTask() {}

	virtual void operator()() {
		if(length <= Cutoff) {
			for(size_t i = 0; i < length; ++i) {
				data[i] += v;
			}
		}
		else {
			size_t half = length >> 1;
			Pheet::template
				spawn_s<Self>(
						Strategy(original_offset, data, half),
						v, original_offset, data, half);
			Pheet::template
				spawn_s<Self>(
						Strategy(original_offset, data + half, length - half),
						v, original_offset, data + half, length - half);
		}
	}
private:
	unsigned int v;
	unsigned int* original_offset;
	unsigned int* data;
	size_t length;
};

} /* namespace pheet */
#endif /* STRATEGYPREFIXSUMOFFSETTASK_H_ */
