/*
 * CircularArrayStealingDequePerformanceCounters.h
 *
 *  Created on: Feb 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef CIRCULARARRAYSTEALINGDEQUEPERFORMANCECOUNTERS_H_
#define CIRCULARARRAYSTEALINGDEQUEPERFORMANCECOUNTERS_H_

#include "../../../settings.h"
#include "../../../primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"

namespace pheet {

template <class Pheet>
class CircularArrayStealingDequePerformanceCounters {
public:
	CircularArrayStealingDequePerformanceCounters() {}
	CircularArrayStealingDequePerformanceCounters(CircularArrayStealingDequePerformanceCounters<Pheet>& other)
		: num_stolen(other.num_stolen),
		  num_pop_cas(other.num_pop_cas) {}
	~CircularArrayStealingDequePerformanceCounters() {}

	static void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, task_storage_count_steals> num_stolen;
	BasicPerformanceCounter<Pheet, task_storage_count_pop_cas> num_pop_cas;
};

template <class Pheet>
inline void CircularArrayStealingDequePerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, task_storage_count_steals>::print_header("stolen\t");
	BasicPerformanceCounter<Pheet, task_storage_count_pop_cas>::print_header("pop_cas\t");
}

template <class Pheet>
inline void CircularArrayStealingDequePerformanceCounters<Pheet>::print_values() {
	num_stolen.print("%lu\t");
	num_pop_cas.print("%lu\t");
}

}

#endif /* CIRCULARARRAYSTEALINGDEQUEPERFORMANCECOUNTERS_H_ */
