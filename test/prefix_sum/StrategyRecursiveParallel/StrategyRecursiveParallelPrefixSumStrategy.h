/*
 * StrategyRecursiveParallelPrefixSumStrategy.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGYRECURSIVEPARALLELPREFIXSUMSTRATEGY_H_
#define STRATEGYRECURSIVEPARALLELPREFIXSUMSTRATEGY_H_

namespace pheet {

template <class Pheet>
class StrategyRecursiveParallelPrefixSumStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef StrategyRecursiveParallelPrefixSumStrategy<Pheet> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	StrategyRecursiveParallelPrefixSumStrategy(unsigned int* data, size_t length, bool in_order, bool no_call)
	:data(data), length(length), in_order(in_order), no_call(no_call) {
		this->set_transitive_weight((length >> 11) + 1);
		this->set_k(1024);
//		place_id = this->get_place()->get_id();
	}
	~StrategyRecursiveParallelPrefixSumStrategy() {}

	inline bool prioritize(Self& other) const {
		if(this->get_place() == Pheet::get_place()) {
			if(this->get_place() == other.get_place()) {
				// Both locally spawned.

				if(in_order) {
					if(other.in_order) {
						pheet_assert(this == &other || data != other.data);
//						last_decision = 1;
						return data < other.data;
					}
//					last_decision = 2;
					return true;
				}
				else if(other.in_order) {
//					last_decision = 3;
					return false;
				}
//				last_decision = 4;
				return length < other.length;
			}
//			last_decision = 5;
			return true;
		}
		else if(other.get_place() == Pheet::get_place()) {
//			last_decision = 6;
			return false;
		}

		if(in_order != other.in_order) {
//			last_decision = 7;
			return other.in_order;
		}
//		last_decision = 8;
		return data > other.data;
	}

	inline bool forbid_call_conversion() const {
		return no_call;
	}

private:
	unsigned int* data;
	size_t length;
	bool in_order;
	bool no_call;
//	size_t place_id;
//	uint8_t last_decision;
};

} /* namespace pheet */
#endif /* STRATEGYRECURSIVEPARALLELPREFIXSUMSTRATEGY_H_ */
