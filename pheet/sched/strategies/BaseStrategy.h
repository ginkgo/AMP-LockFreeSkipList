/*
 * BaseStrategy.h
 *
 *  Created on: 19.09.2011
 *      Author: Martin Wimmer
 */

#ifndef BASESTRATEGY_H_
#define BASESTRATEGY_H_

#include "../../settings.h"
#include "../../misc/types.h"

namespace pheet {

typedef uint64_t prio_t;

template <class Pheet>
class BaseStrategy {
public:
	typedef typename Pheet::Scheduler::StealerDescriptor StealerDescriptor;
	BaseStrategy();
	virtual ~BaseStrategy();

	virtual prio_t get_pop_priority(size_t task_id) = 0;
	virtual prio_t get_steal_priority(size_t task_id, StealerDescriptor& desc) = 0;
	virtual BaseStrategy<Pheet>* clone() = 0;
};

template <class Pheet>
inline BaseStrategy<Pheet>::BaseStrategy() {

}

template <class Pheet>
inline BaseStrategy<Pheet>::~BaseStrategy() {

}

}

#endif /* BASESTRATEGY_H_ */
