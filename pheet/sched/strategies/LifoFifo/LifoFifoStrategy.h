/*
 * LifoFifoStrategy.h
 *
 *  Created on: 21.09.2011
 *      Author: Martin Wimmer
 */

#ifndef LIFOFIFOSTRATEGY_H_
#define LIFOFIFOSTRATEGY_H_

#include "../BaseStrategy.h"
#include <limits>
#include <iostream>

namespace pheet {

template <class Pheet>
class LifoFifoStrategy : public BaseStrategy<Pheet> {
public:
	typedef typename BaseStrategy<Pheet>::StealerDescriptor StealerDescriptor;

	LifoFifoStrategy();
	LifoFifoStrategy(LifoFifoStrategy& other);
	LifoFifoStrategy(LifoFifoStrategy&& other);
	virtual ~LifoFifoStrategy();

	virtual prio_t get_pop_priority(size_t task_id);
	virtual prio_t get_steal_priority(size_t task_id, StealerDescriptor& desc);
	virtual BaseStrategy<Pheet>* clone();

	static void print_name();

private:
};

template <class Pheet>
inline LifoFifoStrategy<Pheet>::LifoFifoStrategy() {

}

template <class Pheet>
inline LifoFifoStrategy<Pheet>::LifoFifoStrategy(LifoFifoStrategy& other) {

}

template <class Pheet>
inline LifoFifoStrategy<Pheet>::LifoFifoStrategy(LifoFifoStrategy&& other) {

}

template <class Pheet>
inline LifoFifoStrategy<Pheet>::~LifoFifoStrategy() {

}

template <class Pheet>
inline prio_t LifoFifoStrategy<Pheet>::get_pop_priority(size_t task_id) {
	return task_id + 1;
}

template <class Pheet>
inline prio_t LifoFifoStrategy<Pheet>::get_steal_priority(size_t task_id, StealerDescriptor& desc) {
	return std::numeric_limits< prio_t >::max() - task_id;
}

template <class Pheet>
inline BaseStrategy<Pheet>* LifoFifoStrategy<Pheet>::clone() {
	return new LifoFifoStrategy<Pheet>(*this);
}

template <class Pheet>
inline void LifoFifoStrategy<Pheet>::print_name() {
	std::cout << "LifoFifoStrategy";
}

}

#endif /* LIFOFIFOSTRATEGY_H_ */
