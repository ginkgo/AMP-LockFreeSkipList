/*
 * CircularArrayStealingDeque11PerformanceCounters.h
 *
 *  Created on: Feb 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef CIRCULARARRAYSTEALINGDEQUE11PERFORMANCECOUNTERS_H_
#define CIRCULARARRAYSTEALINGDEQUE11PERFORMANCECOUNTERS_H_

#include "../../../settings.h"
#include "../../../primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"

namespace pheet {

template <class Pheet>
class CircularArrayStealingDeque11PerformanceCounters {
public:
	CircularArrayStealingDeque11PerformanceCounters() {}
	CircularArrayStealingDeque11PerformanceCounters(CircularArrayStealingDeque11PerformanceCounters<Pheet>& other)
		: num_stolen(other.num_stolen),
		  num_pop_cas(other.num_pop_cas) {}
	~CircularArrayStealingDeque11PerformanceCounters() {}

	static void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, stealing_deque_count_steals> num_stolen;
	BasicPerformanceCounter<Pheet, stealing_deque_count_pop_cas> num_pop_cas;
};

template <class Pheet>
inline void CircularArrayStealingDeque11PerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, stealing_deque_count_steals>::print_header("stolen\t");
	BasicPerformanceCounter<Pheet, stealing_deque_count_pop_cas>::print_header("pop_cas\t");
}

template <class Pheet>
inline void CircularArrayStealingDeque11PerformanceCounters<Pheet>::print_values() {
	num_stolen.print("%lu\t");
	num_pop_cas.print("%lu\t");
}

}

#endif /* CIRCULARARRAYSTEALINGDEQUE11PERFORMANCECOUNTERS_H_ */
