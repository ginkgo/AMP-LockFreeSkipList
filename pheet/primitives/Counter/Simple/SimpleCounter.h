/*
 * SimpleCounter.h
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef SIMPLECOUNTER_H_
#define SIMPLECOUNTER_H_

#include <atomic>
#include <iostream>

namespace pheet {

template <class Pheet, typename T>
class SimpleCounter {
public:
	SimpleCounter()
	:counter(0) {}
	~SimpleCounter() {}

	void incr() {
		++counter;
	}

	T get_sum() {
		return counter;
	}

	static void print_name() {
		std::cout << "SimpleCounter";
	}
private:
	std::atomic<T> counter;
};

} /* namespace pheet */
#endif /* SIMPLECOUNTER_H_ */
