/*
 * LifoStrategy.h
 *
 *  Created on: 22.09.2011
 *      Author: Martin Wimmer
 */

#ifndef LIFOSTRATEGY_H_
#define LIFOSTRATEGY_H_

#include "../BaseStrategy.h"
#include <iostream>

namespace pheet {

template <class Pheet>
class LifoStrategy : public BaseStrategy<Pheet> {
public:
	typedef typename BaseStrategy<Pheet>::StealerDescriptor StealerDescriptor;

	LifoStrategy();
	LifoStrategy(LifoStrategy& other);
	LifoStrategy(LifoStrategy&& other);
	virtual ~LifoStrategy();

	virtual prio_t get_pop_priority(size_t task_id);
	virtual prio_t get_steal_priority(size_t task_id, StealerDescriptor& desc);
	virtual BaseStrategy<Pheet>* clone();

	static void print_name();
};

template <class Pheet>
inline LifoStrategy<Pheet>::LifoStrategy() {

}

template <class Pheet>
inline LifoStrategy<Pheet>::LifoStrategy(LifoStrategy& other) {

}

template <class Pheet>
inline LifoStrategy<Pheet>::LifoStrategy(LifoStrategy&& other) {

}

template <class Pheet>
inline LifoStrategy<Pheet>::~LifoStrategy() {

}

template <class Pheet>
inline prio_t LifoStrategy<Pheet>::get_pop_priority(size_t task_id) {
	return task_id + 1;
}

template <class Pheet>
inline prio_t LifoStrategy<Pheet>::get_steal_priority(size_t task_id, StealerDescriptor& desc) {
	return task_id + 1;
}

template <class Pheet>
inline BaseStrategy<Pheet>* LifoStrategy<Pheet>::clone() {
	return new LifoStrategy<Pheet>(*this);
}

template <class Pheet>
inline void LifoStrategy<Pheet>::print_name() {
	std::cout << "LifoStrategy";
}

}

#endif /* LIFOSTRATEGY_H_ */
