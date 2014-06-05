/*
 * LUPivLocalityStrategy.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPLUPIVLOCALITYSTRATEGY_H_
#define PPOPPLUPIVLOCALITYSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/BaseStrategy.h>

namespace pheet {

template <class Pheet, bool s2c>
  class PPoPPLUPivLocalityStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef PPoPPLUPivLocalityStrategy<Pheet, s2c> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	PPoPPLUPivLocalityStrategy(typename Pheet::Place* last_owner, size_t blocks, bool critical_path);
	PPoPPLUPivLocalityStrategy(Self& other);
	PPoPPLUPivLocalityStrategy(Self&& other);
	~PPoPPLUPivLocalityStrategy();

	inline bool prioritize(Self& other);

	inline bool forbid_call_conversion() const {
		return !s2c;
	}

private:
	typename Pheet::Place* last_owner;
	size_t blocks;
	bool critical_path;
};

template <class Pheet, bool s2c>
PPoPPLUPivLocalityStrategy<Pheet, s2c>::PPoPPLUPivLocalityStrategy(typename Pheet::Place* last_owner, size_t blocks, bool critical_path)
  : last_owner(last_owner), blocks(blocks), critical_path(critical_path) {
	this->set_transitive_weight(blocks << 8);
}

template <class Pheet, bool s2c>
PPoPPLUPivLocalityStrategy<Pheet, s2c>::PPoPPLUPivLocalityStrategy(Self& other)
  : BaseStrategy(other),last_owner(other.last_owner), blocks(other.blocks), critical_path(other.critical_path) {

}

template <class Pheet, bool s2c>
PPoPPLUPivLocalityStrategy<Pheet, s2c>::PPoPPLUPivLocalityStrategy(Self&& other)
  : BaseStrategy(other),last_owner(other.last_owner), blocks(other.blocks), critical_path(other.critical_path) {

}

template <class Pheet, bool s2c>
PPoPPLUPivLocalityStrategy<Pheet, s2c>::~PPoPPLUPivLocalityStrategy() {

}


template <class Pheet, bool s2c>
inline bool PPoPPLUPivLocalityStrategy<Pheet, s2c>::prioritize(Self& other) {
	typename Pheet::Place* p = Pheet::get_place();
	procs_t d = p->get_distance(last_owner);
	procs_t d_o = p->get_distance(other.last_owner);

	if(d != d_o) {
		return d < d_o;
	}
	if(this->get_place() == p) {
		if(other.get_place() == p) {
			if(blocks != other.blocks) {
				return blocks < other.blocks;
			}
			return critical_path && (!other.critical_path);
		}
		return true;
	}
	else if(other.get_place() == p) {
		return true;
	}
	if(blocks != other.blocks) {
		return blocks > other.blocks;
	}
	return critical_path && (!other.critical_path);
}

}

#endif /* PPOPPLUPIVLOCALITYSTRATEGY_H_ */
