/*
 * StrategyQuicksortStrategy.h
 *
 *  Created on: 03.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGY2QUICKSORTSTRATEGY_H_
#define STRATEGY2QUICKSORTSTRATEGY_H_

#include <pheet/misc/bitops.h>

#include <pheet/ds/StrategyTaskStorage/LSMLocality/LSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet>
class Strategy2QuicksortStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef Strategy2QuicksortStrategy<Pheet> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	typedef LSMLocalityTaskStorage<Pheet, Self> TaskStorage;
//	typedef typename Pheet::Environment::TaskStorage TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	/*
	 * Default constructor is required by task storage
	 */
	Strategy2QuicksortStrategy() {

	}

	Strategy2QuicksortStrategy(size_t length)
	: length(length), place(Pheet::get_place()) {

	}

	Strategy2QuicksortStrategy(Self const& other) = default;

	Strategy2QuicksortStrategy(Self&& other) = default;

	~Strategy2QuicksortStrategy() {}

	Self& operator=(Self&& other) {
//		BaseStrategy::operator=(other);
		length = other.length;
		place = other.place;
		return *this;
	}

	bool prioritize(Self& other) {
		Place* p = Pheet::get_place();
		if(this->place == p) {
			if(other.place == p) {
				return length < other.length;
			}
			return true;
		}
		else if(other.place == p) {
			return false;
		}
		return length > other.length;
	}

	bool dead_task() {
		return false;
	}

	/*
	 * Checks whether spawn can be converted to a function call
	 */
	inline bool can_call(TaskStoragePlace*) {
		// Due to the static cutoff in the algorithm, the gains of spawn2call are minimal
		// so we omit this overhead
		return false;
	//	return (length / (tsp->size() + 1)) < 512;
	}

private:
	size_t length;
	Place* place;
};

}

#endif /* STRATEGYQUICKSORTSTRATEGY_H_ */
