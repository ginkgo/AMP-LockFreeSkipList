/*
 * VolatileStrategyHeap2PerformanceCounters.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef VOLATILESTRATEGYHEAP2PERFORMANCECOUNTERS_H_
#define VOLATILESTRATEGYHEAP2PERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet>
class VolatileStrategyHeap2PerformanceCounters {
public:
	typedef VolatileStrategyHeap2PerformanceCounters<Pheet> Self;

	VolatileStrategyHeap2PerformanceCounters();
	VolatileStrategyHeap2PerformanceCounters(Self& other);
	~VolatileStrategyHeap2PerformanceCounters();

	static void print_headers();
	void print_values();
};


template <class Pheet>
inline VolatileStrategyHeap2PerformanceCounters<Pheet>::VolatileStrategyHeap2PerformanceCounters() {

}

template <class Pheet>
VolatileStrategyHeap2PerformanceCounters<Pheet>::VolatileStrategyHeap2PerformanceCounters(VolatileStrategyHeap2PerformanceCounters&)
{

}

template <class Pheet>
inline VolatileStrategyHeap2PerformanceCounters<Pheet>::~VolatileStrategyHeap2PerformanceCounters() {

}

template <class Pheet>
void VolatileStrategyHeap2PerformanceCounters<Pheet>::print_headers() {
}

template <class Pheet>
void VolatileStrategyHeap2PerformanceCounters<Pheet>::print_values() {

}

}

#endif /* VOLATILESTRATEGYHEAP2PERFORMANCECOUNTERS_H_ */
