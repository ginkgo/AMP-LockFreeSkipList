/*
 * LUPivLocalityStrategy.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LUPIVLOCALITYSTRATEGY_H_
#define LUPIVLOCALITYSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/BaseStrategy.h>

namespace pheet {

template <class Pheet>
class LUPivLocalityStrategy : public BaseStrategy<Pheet> {
public:
	typedef LUPivLocalityStrategy<Pheet> Self;
	LUPivLocalityStrategy(typename Pheet::Place* last_owner, prio_t base_pop_priority, prio_t base_steal_priority);
	LUPivLocalityStrategy(Self& other);
	LUPivLocalityStrategy(Self&& other);
	virtual ~LUPivLocalityStrategy();

	virtual prio_t get_pop_priority(size_t task_id);
	virtual prio_t get_steal_priority(size_t task_id, typename Pheet::Scheduler::StealerDescriptor& desc);
	virtual BaseStrategy<Pheet>* clone();

private:
	typename Pheet::Place* last_owner;
	prio_t base_pop_priority;
	prio_t base_steal_priority;
};

template <class Pheet>
LUPivLocalityStrategy<Pheet>::LUPivLocalityStrategy(typename Pheet::Place* last_owner, prio_t base_pop_priority, prio_t base_steal_priority)
: last_owner(last_owner), base_pop_priority(base_pop_priority), base_steal_priority(base_steal_priority) {
	pheet_assert(base_pop_priority < 16);
	pheet_assert(base_steal_priority < 16);
}

template <class Pheet>
LUPivLocalityStrategy<Pheet>::LUPivLocalityStrategy(Self& other)
: last_owner(other.last_owner), base_pop_priority(other.base_pop_priority), base_steal_priority(other.base_steal_priority) {

}

template <class Pheet>
LUPivLocalityStrategy<Pheet>::LUPivLocalityStrategy(Self&& other)
: last_owner(other.last_owner), base_pop_priority(other.base_pop_priority), base_steal_priority(other.base_steal_priority) {

}

template <class Pheet>
LUPivLocalityStrategy<Pheet>::~LUPivLocalityStrategy() {

}

template <class Pheet>
inline prio_t LUPivLocalityStrategy<Pheet>::get_pop_priority(size_t task_id) {
/*	typename Pheet::TaskExecutionContext* owner = Pheet::get_context();
	procs_t max_d = owner->get_max_distance();
	procs_t d = owner->get_distance(last_owner);
	return ((max_d - d) << 4) + base_pop_priority;*/
	return (base_pop_priority << 6) + task_id;
}

template <class Pheet>
inline prio_t LUPivLocalityStrategy<Pheet>::get_steal_priority(size_t task_id, typename Pheet::Scheduler::StealerDescriptor& desc) {
	procs_t max_d = desc.get_max_distance();
	procs_t d = desc.get_distance_to(last_owner);
	return ((((max_d - d) << 4) + base_steal_priority) << 16) - (task_id % (1 << 16));
}

template <class Pheet>
inline BaseStrategy<Pheet>* LUPivLocalityStrategy<Pheet>::clone() {
	return new Self(*this);
}

}

#endif /* LUPIVLOCALITYSTRATEGY_H_ */
