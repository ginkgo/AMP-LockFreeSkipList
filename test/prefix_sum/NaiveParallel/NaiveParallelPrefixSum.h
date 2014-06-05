/*
 * NaiveParallelPrefixSum.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef NAIVEPARALLELPREFIXSUM_H_
#define NAIVEPARALLELPREFIXSUM_H_

#include <pheet/pheet.h>
#include "NaiveParallelPrefixSumOffsetTask.h"

namespace pheet {

template <class Pheet, size_t Cutoff>
class NaiveParallelPrefixSumImpl : public Pheet::Task {
public:
	typedef NaiveParallelPrefixSumImpl<Pheet, Cutoff> Self;
	typedef NaiveParallelPrefixSumOffsetTask<Pheet, Cutoff> OffsetTask;

	NaiveParallelPrefixSumImpl(unsigned int* data, size_t length)
	:data(data), length(length) {

	}
	virtual ~NaiveParallelPrefixSumImpl() {}

	virtual void operator()() {
		if(length <= Cutoff) {
			for(size_t i = 1; i < length; ++i) {
				data[i] += data[i - 1];
			}
		}
		else {
			size_t half = length >> 1;
			{
				typename Pheet::Finish f;
				Pheet::template
					spawn<Self>(data, half);
				Pheet::template
					spawn<Self>(data + half, length - half);
			}

			Pheet::template
				call<OffsetTask>(data[half - 1], data + half, length - half);
		}
	}

	static char const name[];

private:
	unsigned int* data;
	size_t length;
};

template <class Pheet, size_t Cutoff>
char const NaiveParallelPrefixSumImpl<Pheet, Cutoff>::name[] = "NaiveParallelPrefixSum";

template <class Pheet>
using NaiveParallelPrefixSum = NaiveParallelPrefixSumImpl<Pheet, 1024>;

} /* namespace pheet */
#endif /* NAIVEPARALLELPREFIXSUM_H_ */
