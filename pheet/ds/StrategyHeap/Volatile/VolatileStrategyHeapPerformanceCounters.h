/*
 * VolatileStrategyHeapPerformanceCounters.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef VOLATILESTRATEGYHEAPPERFORMANCECOUNTERS_H_
#define VOLATILESTRATEGYHEAPPERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet>
class VolatileStrategyHeapPerformanceCounters {
public:
	typedef VolatileStrategyHeapPerformanceCounters<Pheet> Self;

	VolatileStrategyHeapPerformanceCounters();
	VolatileStrategyHeapPerformanceCounters(Self& other);
	~VolatileStrategyHeapPerformanceCounters();

	static void print_headers();
	void print_values();
};


template <class Pheet>
inline VolatileStrategyHeapPerformanceCounters<Pheet>::VolatileStrategyHeapPerformanceCounters() {

}

template <class Pheet>
VolatileStrategyHeapPerformanceCounters<Pheet>::VolatileStrategyHeapPerformanceCounters(VolatileStrategyHeapPerformanceCounters& other)
{

}

template <class Pheet>
inline VolatileStrategyHeapPerformanceCounters<Pheet>::~VolatileStrategyHeapPerformanceCounters() {

}

template <class Pheet>
void VolatileStrategyHeapPerformanceCounters<Pheet>::print_headers() {
}

template <class Pheet>
void VolatileStrategyHeapPerformanceCounters<Pheet>::print_values() {

}

}

#endif /* VOLATILESTRATEGYHEAPPERFORMANCECOUNTERS_H_ */
