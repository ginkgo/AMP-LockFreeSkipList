/*
 * StrategyPrefixSum.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGYPREFIXSUM_H_
#define STRATEGYPREFIXSUM_H_

#include <pheet/pheet.h>
#include "StrategyPrefixSumOffsetTask.h"
#include "StrategyPrefixSumStrategy.h"

namespace pheet {

template <class Pheet, size_t Cutoff>
class StrategyPrefixSumImpl : public Pheet::Task {
public:
	typedef StrategyPrefixSumImpl<Pheet, Cutoff> Self;
	typedef StrategyPrefixSumOffsetTask<Pheet, Cutoff> OffsetTask;
	typedef StrategyPrefixSumStrategy<Pheet> Strategy;

	StrategyPrefixSumImpl(unsigned int* data, size_t length)
	:original_offset(data), data(data), length(length), state(nullptr), second(false) {

	}

	StrategyPrefixSumImpl(unsigned int* original_offset, unsigned int* data, size_t length, uint8_t& state, bool second)
	:original_offset(original_offset), data(data), length(length), state(&state), second(second) {

	}
	virtual ~StrategyPrefixSumImpl() {}

	virtual void operator()() {
		uint8_t s = 0;
		if(second) {
			s = *state;
			if(s == 1) {
				*state = 2;
				*data += *(data - 1);
			}
		}
		if(length <= Cutoff) {
			for(size_t i = 1; i < length; ++i) {
				data[i] += data[i - 1];
			}
		}
		else {
			size_t half = length >> 1;
			uint8_t sub_state = 0;
			{
				typename Pheet::Finish f;
				Pheet::template
					spawn_s<Self>(
							Strategy(original_offset, data, half, sub_state, false),
							original_offset, data, half, sub_state, false);
				Pheet::template
					spawn_s<Self>(
							Strategy(original_offset, data + half, length - half, sub_state, true),
							original_offset, data + half, length - half, sub_state, true);
			}
			pheet_assert(sub_state >= 1);
			if(sub_state != 2) {
				Pheet::template
					finish<OffsetTask>(
							data[half - 1], original_offset, data + half, length - half);
			}
		}
		if(state != nullptr && !second) {
			MEMORY_FENCE();
			*state = 1;
		}
	}

	static char const name[];

private:
	unsigned int* original_offset;
	unsigned int* data;
	size_t length;
	uint8_t* state;
	bool second;
};

template <class Pheet, size_t Cutoff>
char const StrategyPrefixSumImpl<Pheet, Cutoff>::name[] = "StrategyPrefixSum";

template <class Pheet>
using StrategyPrefixSum = StrategyPrefixSumImpl<Pheet, 1024>;

} /* namespace pheet */
#endif /* STRATEGYPREFIXSUM_H_ */
