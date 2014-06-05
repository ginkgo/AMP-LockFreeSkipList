/*
 * BasicStrategyHeapPerformanceCounters.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICSTRATEGYHEAPPERFORMANCECOUNTERS_H_
#define BASICSTRATEGYHEAPPERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet>
class BasicStrategyHeapPerformanceCounters {
public:
	typedef BasicStrategyHeapPerformanceCounters<Pheet> Self;

	BasicStrategyHeapPerformanceCounters();
	BasicStrategyHeapPerformanceCounters(Self& other);
	~BasicStrategyHeapPerformanceCounters();

	static void print_headers();
	void print_values();
};


template <class Pheet>
inline BasicStrategyHeapPerformanceCounters<Pheet>::BasicStrategyHeapPerformanceCounters() {

}

template <class Pheet>
BasicStrategyHeapPerformanceCounters<Pheet>::BasicStrategyHeapPerformanceCounters(BasicStrategyHeapPerformanceCounters&)
{

}

template <class Pheet>
inline BasicStrategyHeapPerformanceCounters<Pheet>::~BasicStrategyHeapPerformanceCounters() {

}

template <class Pheet>
void BasicStrategyHeapPerformanceCounters<Pheet>::print_headers() {
}

template <class Pheet>
void BasicStrategyHeapPerformanceCounters<Pheet>::print_values() {

}

}

#endif /* BASICSTRATEGYHEAPPERFORMANCECOUNTERS_H_ */
