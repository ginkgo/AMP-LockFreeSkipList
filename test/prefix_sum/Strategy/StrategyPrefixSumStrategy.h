/*
 * StrategyPrefixSumStrategy.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGYPREFIXSUMSTRATEGY_H_
#define STRATEGYPREFIXSUMSTRATEGY_H_

namespace pheet {

template <class Pheet>
class StrategyPrefixSumStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef StrategyPrefixSumStrategy<Pheet> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	StrategyPrefixSumStrategy(unsigned int* original_offset, unsigned int* data, size_t length, uint8_t& state, bool second)
	: original_offset(original_offset), data(data), length(length), prefer_local(!second || state >= 1), prefer_remote(false) {
	//	this->set_memory_footprint(length);
		this->set_transitive_weight(1 + (length >> 9));
	}

	StrategyPrefixSumStrategy(unsigned int* original_offset, unsigned int* data, size_t length)
	: original_offset(original_offset), data(data), length(length), prefer_local(true), prefer_remote(true) {
	//	this->set_memory_footprint(length);
		this->set_transitive_weight(1 + (length >> 9));
	}


	StrategyPrefixSumStrategy(Self const& other)
	: BaseStrategy(other), original_offset(other.original_offset), data(other.data), length(other.length), prefer_local(other.prefer_local), prefer_remote(other.prefer_remote) {}

	StrategyPrefixSumStrategy(Self&& other)
	: BaseStrategy(other), original_offset(other.original_offset), data(other.data), length(other.length), prefer_local(other.prefer_local), prefer_remote(other.prefer_remote) {}

	~StrategyPrefixSumStrategy() {}

	inline bool prioritize(Self& other) {
		if(this->get_place() == other.get_place() && this->get_place() == Pheet::get_place()) {
			// Locally spawned task.

			if(prefer_local != other.prefer_local) {
				return prefer_local;
			}
/*
			if(original_offset == other.original_offset) {
				return data < other.data;
			}*/
			return (data - original_offset) < (other.data - other.original_offset);
		}
		if(prefer_remote != other.prefer_remote) {
			return prefer_remote;
		}

		return data > other.data;
	}

	inline bool forbid_call_conversion() {
		return prefer_local;
	}

private:
	unsigned int* original_offset;
	unsigned int* data;
	size_t length;
	bool prefer_local;
	bool prefer_remote;
};


} /* namespace pheet */
#endif /* STRATEGYPREFIXSUMSTRATEGY_H_ */
