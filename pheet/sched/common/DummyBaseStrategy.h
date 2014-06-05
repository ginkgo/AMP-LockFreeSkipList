/*
 * DummyBaseStrategy.h
 *
 *  Created on: 21.06.2012
 *      Author: mwimmer
 */

#ifndef DUMMYBASESTRATEGY_H_
#define DUMMYBASESTRATEGY_H_

namespace pheet {

template <class Pheet>
class DummyBaseStrategy {
public:
	typedef DummyBaseStrategy<Pheet> Self;
	typedef typename Pheet::Place Place;

	DummyBaseStrategy()
	: transitive_weight(1024)/*, memory_footprint(1024)*/ {}

	DummyBaseStrategy(Self const& other)
	: transitive_weight(other.transitive_weight) {}

	DummyBaseStrategy(Self&& other)
	: transitive_weight(other.transitive_weight) {}

	~DummyBaseStrategy() {}

	inline bool prioritize(Self&) {
		return false;
	}

	inline Place* get_place() {
		return Pheet::get_place();
	}

	inline size_t get_transitive_weight() {
		return transitive_weight;
	}
/*
	inline size_t get_memory_footprint() {
		return memory_footprint;
	}*/

	inline Self& set_transitive_weight(size_t value) {
		transitive_weight = value;
		return *this;
	}

	inline void set_k(size_t) {
	}

	inline Self& set_memory_footprint(size_t) {
	//	memory_footprint = value;
		return *this;
	}
private:
	size_t transitive_weight;
};

} /* namespace pheet */
#endif /* DUMMYBASESTRATEGY_H_ */
