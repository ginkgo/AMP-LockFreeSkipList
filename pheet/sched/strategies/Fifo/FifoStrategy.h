/*
 * FifoStrategy.h
 *
 *  Created on: 22.09.2011
 *      Author: Martin Wimmer
 */

#ifndef FIFOSTRATEGY_H_
#define FIFOSTRATEGY_H_

#include "../BaseStrategy.h"
#include <limits>
#include <iostream>

namespace pheet {

template <class Pheet>
class FifoStrategy : public BaseStrategy<Pheet> {
public:
	typedef typename BaseStrategy<Pheet>::StealerDescriptor StealerDescriptor;

	FifoStrategy();
	FifoStrategy(FifoStrategy& other);
	FifoStrategy(FifoStrategy&& other);
	virtual ~FifoStrategy();

	virtual prio_t get_pop_priority(size_t task_id);
	virtual prio_t get_steal_priority(size_t task_id, StealerDescriptor& desc);
	virtual BaseStrategy<Pheet>* clone();

	static void print_name();
};

template <class Pheet>
inline FifoStrategy<Pheet>::FifoStrategy() {

}

template <class Pheet>
inline FifoStrategy<Pheet>::FifoStrategy(FifoStrategy& other) {

}

template <class Pheet>
inline FifoStrategy<Pheet>::FifoStrategy(FifoStrategy&& other) {

}

template <class Pheet>
inline FifoStrategy<Pheet>::~FifoStrategy() {

}

template <class Pheet>
inline prio_t FifoStrategy<Pheet>::get_pop_priority(size_t task_id) {
	return std::numeric_limits< prio_t >::max() - task_id;
}

template <class Pheet>
inline prio_t FifoStrategy<Pheet>::get_steal_priority(size_t task_id, StealerDescriptor& desc) {
	return std::numeric_limits< prio_t >::max() - task_id;
}

template <class Pheet>
inline BaseStrategy<Pheet>* FifoStrategy<Pheet>::clone() {
	return new FifoStrategy<Pheet>(*this);
}

template <class Pheet>
inline void FifoStrategy<Pheet>::print_name() {
	std::cout << "FifoStrategy";
}

}

#endif /* FIFOSTRATEGY_H_ */
