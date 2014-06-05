/*
 * SequentialPrefixSum.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SEQUENTIALPREFIXSUM_H_
#define SEQUENTIALPREFIXSUM_H_

#include <pheet/pheet.h>
#include <pheet/primitives/PerformanceCounter/DummyPerformanceCounters.h>

namespace pheet {

template <class Pheet>
class SequentialPrefixSum : public Pheet::Task {
public:
	typedef DummyPerformanceCounters<Pheet> PerformanceCounters;

	SequentialPrefixSum(unsigned int* data, size_t length)
	:data(data), length(length) {

	}
	SequentialPrefixSum(unsigned int* data, size_t length, PerformanceCounters&p)
		:data(data), length(length) {}
	virtual ~SequentialPrefixSum() {}

	virtual void operator()() {
		for(size_t i = 1; i < length; ++i) {
			data[i] += data[i - 1];
		}
	}

	static char const name[];

private:
	unsigned int* data;
	size_t length;
};

template <class Pheet>
char const SequentialPrefixSum<Pheet>::name[] = "SequentialPrefixSum";

} /* namespace pheet */
#endif /* SEQUENTIALPREFIXSUM_H_ */
