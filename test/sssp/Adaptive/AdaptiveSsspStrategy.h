/*
 * AdaptiveSsspStrategy.h
 *
 *  Created on: Jul 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef ADAPTIVESSSPSTRATEGY_H_
#define ADAPTIVESSSPSTRATEGY_H_

#include <limits>
#include <atomic>

namespace pheet {

template <class Pheet>
class AdaptiveSsspStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef AdaptiveSsspStrategy<Pheet> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	AdaptiveSsspStrategy(size_t distance, std::atomic<size_t>& stored_distance, size_t k)
	: distance(distance), stored_distance(stored_distance)/*, rnd(Pheet::rand_int((1 + distance) << 4))*/ {
		this->set_k(k);
	}

	AdaptiveSsspStrategy(Self& other)
	: BaseStrategy(other), distance(other.distance), stored_distance(other.stored_distance)/*, rnd(other.rnd)*/ {
		this->set_k(other.get_k());
	}

	AdaptiveSsspStrategy(Self&& other)
	: BaseStrategy(other), distance(other.distance), stored_distance(other.stored_distance)/*, rnd(other.rnd)*/ {}

	~AdaptiveSsspStrategy() {}

	inline bool prioritize(Self& other) const {
/*		auto p = Pheet::get_place();
		if(this->get_place() == p) {
			if(other.get_place() == p) {*/
				return distance < other.distance;
/*			}
			return true;
		}
		else if(other.get_place() == p) {
			return false;
		}
		return rnd < other.rnd;*/

	//	return BaseAdaptive::prioritize(other);
	}

	inline bool forbid_call_conversion() const {
		return true;
	}

	inline bool dead_task() {
		return stored_distance.load(std::memory_order_relaxed) < distance;
	}
/*
	inline void rebase() {
		this->reset();
		rnd = Pheet::rand_int((1 + distance) << 4);
	}*/
private:
	size_t distance;
	std::atomic<size_t>& stored_distance;
//	size_t rnd;
};


} /* namespace pheet */
#endif /* ADAPTIVESSSPSTRATEGY_H_ */
