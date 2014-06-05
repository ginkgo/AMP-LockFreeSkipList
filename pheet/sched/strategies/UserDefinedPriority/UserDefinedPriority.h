/*
 * UserDefinedPriority.h
 *
 *  Created on: 21.09.2011
 *      Author: Martin Wimmer
 */

#ifndef USERDEFINEDPRIORITY_H_
#define USERDEFINEDPRIORITY_H_

#include "../BaseStrategy.h"

namespace pheet {

template <class Pheet>
class UserDefinedPriority : public BaseStrategy<Pheet> {
public:
	typedef typename BaseStrategy<Pheet>::StealerDescriptor StealerDescriptor;

	UserDefinedPriority(prio_t pop_priority, prio_t steal_priority);
	UserDefinedPriority(UserDefinedPriority& other);
	UserDefinedPriority(UserDefinedPriority&& other);
	virtual ~UserDefinedPriority();

	virtual prio_t get_pop_priority(size_t task_id);
	virtual prio_t get_steal_priority(size_t task_id, StealerDescriptor& desc);
	virtual BaseStrategy<Pheet>* clone();
private:
	prio_t pop_priority;
	prio_t steal_priority;
};

template <class Pheet>
inline UserDefinedPriority<Pheet>::UserDefinedPriority(prio_t pop_priority, prio_t steal_priority)
: pop_priority(pop_priority), steal_priority(steal_priority) {

}

template <class Pheet>
inline UserDefinedPriority<Pheet>::UserDefinedPriority(UserDefinedPriority& other)
: pop_priority(other.pop_priority), steal_priority(other.steal_priority) {

}

template <class Pheet>
inline UserDefinedPriority<Pheet>::UserDefinedPriority(UserDefinedPriority&& other)
: pop_priority(other.pop_priority), steal_priority(other.steal_priority) {

}

template <class Pheet>
inline UserDefinedPriority<Pheet>::~UserDefinedPriority() {

}

template <class Pheet>
inline prio_t UserDefinedPriority<Pheet>::get_pop_priority(size_t task_id) {
	return pop_priority;
}

template <class Pheet>
inline prio_t UserDefinedPriority<Pheet>::get_steal_priority(size_t task_id, StealerDescriptor& desc) {
	return steal_priority;
}

template <class Pheet>
inline BaseStrategy<Pheet>* UserDefinedPriority<Pheet>::clone() {
	return new UserDefinedPriority<Pheet>(*this);
}

}

#endif /* USERDEFINEDPRIORITY_H_ */
