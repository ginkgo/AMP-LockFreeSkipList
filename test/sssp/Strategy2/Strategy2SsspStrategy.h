/*
 * Strategy2SsspStrategy.h
 *
 *  Created on: Oct 08, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGY2SSSPSTRATEGY_H_
#define STRATEGY2SSSPSTRATEGY_H_

#include <limits>
#include <atomic>

#include <pheet/ds/StrategyTaskStorage/KLSMLocality/KLSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet, template <class, class> class TaskStorageT>
class Strategy2SsspStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef Strategy2SsspStrategy<Pheet, TaskStorageT> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

//	typedef KLSMLocalityTaskStorage<Pheet, Self> TaskStorage;
	typedef TaskStorageT<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	Strategy2SsspStrategy() {

	}

	Strategy2SsspStrategy(size_t distance, std::atomic<size_t>& stored_distance)
	: distance(distance), stored_distance(&stored_distance)/*, rnd(Pheet::rand_int((1 + distance) << 4))*/ {
//		this->set_k(default_k);
	}

	Strategy2SsspStrategy(Self& other)
	: BaseStrategy(other), distance(other.distance), stored_distance(other.stored_distance)/*, rnd(other.rnd)*/ {

	}

	Strategy2SsspStrategy(Self&& other)
	: BaseStrategy(other), distance(other.distance), stored_distance(other.stored_distance)/*, rnd(other.rnd)*/ {}

	~Strategy2SsspStrategy() {}

	Self& operator=(Self&& other) {
//		BaseStrategy::operator=(other);
		distance = other.distance;
		stored_distance = other.stored_distance;
		return *this;
	}

	bool prioritize(Self& other) {
		return distance < other.distance;
	}

	bool dead_task() {
		return stored_distance->load(std::memory_order_relaxed) < distance;
	}

	/*
	 * Checks whether spawn can be converted to a function call
	 */
	inline bool can_call(TaskStoragePlace*) {
		// Call conversion would worsen complexity of algorithm
		return false;
	}

	inline bool forbid_call_conversion() const {
		return true;
	}

	inline size_t get_k() {
		return default_k;
	}

	static size_t default_k;
private:
	size_t distance;
	std::atomic<size_t>* stored_distance;
//	size_t rnd;
};

template <class Pheet, template <class, class> class TaskStorageT>
size_t Strategy2SsspStrategy<Pheet, TaskStorageT>::default_k = 1024;

} /* namespace pheet */
#endif /* STRATEGY2SSSPSTRATEGY_H_ */
