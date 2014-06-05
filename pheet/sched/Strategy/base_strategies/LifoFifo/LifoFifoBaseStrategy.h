/*
 * LifoFifoBaseStrategy.h
 *
 *  Created on: 03.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LIFOFIFOBASESTRATEGY_H_
#define LIFOFIFOBASESTRATEGY_H_

#include <limits>

namespace pheet {

template <class Pheet>
class LifoFifoBaseStrategy {
public:
	typedef LifoFifoBaseStrategy<Pheet> Self;
	typedef typename Pheet::Place Place;

	LifoFifoBaseStrategy()
	: place(Pheet::get_place()), task_id(place->next_task_id()), transitive_weight(64), k(0x10000000)/*, memory_footprint(1024)*/ {
	//	pheet_assert(transitive_weight != 0);
		pheet_assert(transitive_weight < ((std::numeric_limits<size_t>::max() >> 3)));
	}

	LifoFifoBaseStrategy(Self const& other)
	: place(other.place), task_id(other.task_id), transitive_weight(other.transitive_weight), k(other.k)/*, memory_footprint(other.memory_footprint)*/ {
	//	pheet_assert(transitive_weight != 0 || !other.forbid_call_conversion());
	}

	LifoFifoBaseStrategy(Self&& other)
	: place(other.place), task_id(other.task_id), transitive_weight(other.transitive_weight), k(other.k)/*, memory_footprint(other.memory_footprint)*/ {
	//	pheet_assert(transitive_weight != 0 || !other.forbid_call_conversion());
		pheet_assert(transitive_weight < ((std::numeric_limits<size_t>::max() >> 5)));
	}

	virtual ~LifoFifoBaseStrategy() {}

	inline bool prioritize(Self& other) const {
		Place* cur_place = Pheet::get_place();
		if(other.place == place) {
			if(place == cur_place) {
				return (task_id - other.task_id) > 0;
			}
			else {
				return (other.task_id - task_id) > 0;
			}
		}
		else {
			size_t d1 = cur_place->get_distance(place);
			size_t d2 = cur_place->get_distance(other.place);
			if(d1 != d2) {
				return d1 < d2;
			}
			else {
				// We have to prioritize some place to keep the ordering property, although we don't actually care
				// Ideally we might have a different ordering per cur_place to reduce congestion?
				return place > other.place;
			}
		}
	}

	inline size_t get_task_id() const {
		return task_id;
	}

	inline size_t new_task_id() {
		return task_id = Pheet::get_place()->next_task_id();
	}

	inline void set_place(Place* place) {
		this->place = place;
	}

	inline Place* get_place() const {
		return place;
	}

	inline size_t get_transitive_weight() const {
		return transitive_weight;
	}
/*
	inline size_t get_memory_footprint() {
		return memory_footprint;
	}*/

	inline Self& set_transitive_weight(size_t value) {
	//	pheet_assert(value != 0 || !forbid_call_conversion());
		pheet_assert(value < ((std::numeric_limits<size_t>::max() >> 3)));
		transitive_weight = value;
		return *this;
	}

	inline bool forbid_call_conversion() const {
		return true;
	}

	inline void rebase() {}

	inline void reset() {
		set_place(Pheet::get_place());
		new_task_id();
	}

	inline size_t get_k() {
		return k;
	}

	inline void set_k(size_t k) {
		this->k = k;
	}

	inline bool dead_task() {
		return false;
	}

/*
	inline Self& set_memory_footprint(size_t value) {
		memory_footprint = value;
		return *this;
	}*/
private:
	Place* place;
	ptrdiff_t task_id;
	size_t transitive_weight;
	size_t k;
//	size_t memory_footprint;
};

}

#endif /* LIFOFIFOBASESTRATEGY_H_ */
