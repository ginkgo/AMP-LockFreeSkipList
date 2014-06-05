/*
 * StrategyQuicksortStrategy.h
 *
 *  Created on: 03.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYQUICKSORTSTRATEGY_H_
#define STRATEGYQUICKSORTSTRATEGY_H_

#include <pheet/misc/bitops.h>

namespace pheet {

template <class Pheet>
class StrategyQuicksortStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef StrategyQuicksortStrategy<Pheet> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	StrategyQuicksortStrategy(size_t length)
	: length(length) {
	//	this->set_memory_footprint(length);
		size_t l2 = length >> 10;
		this->set_transitive_weight(l2 * find_last_bit_set(length));
	}

	StrategyQuicksortStrategy(Self& other)
	: BaseStrategy(other), length(other.length) {}

	StrategyQuicksortStrategy(Self&& other)
	: BaseStrategy(other), length(other.length) {}

	~StrategyQuicksortStrategy() {}

	inline bool prioritize(Self& other) const {
		if(this->get_place() == other.get_place() && this->get_place() == Pheet::get_place()) {
			return length < other.length;
		}
		return length > other.length;
	}

	inline bool forbid_call_conversion() const {
		return false;
	}

private:
	size_t length;
};

}

#endif /* STRATEGYQUICKSORTSTRATEGY_H_ */
