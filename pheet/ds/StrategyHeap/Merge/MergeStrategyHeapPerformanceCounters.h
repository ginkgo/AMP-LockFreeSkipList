/*
 * MergeStrategyHeapPerformanceCounters.h
 *
 *  Created on: Mar 08, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef MERGESTRATEGYHEAPPERFORMANCECOUNTERS_H_
#define MERGESTRATEGYHEAPPERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet>
class MergeStrategyHeapPerformanceCounters {
public:
	typedef MergeStrategyHeapPerformanceCounters<Pheet> Self;

	MergeStrategyHeapPerformanceCounters();
	MergeStrategyHeapPerformanceCounters(Self& other);
	~MergeStrategyHeapPerformanceCounters();

	static void print_headers();
	void print_values();
};


template <class Pheet>
inline MergeStrategyHeapPerformanceCounters<Pheet>::MergeStrategyHeapPerformanceCounters() {

}

template <class Pheet>
MergeStrategyHeapPerformanceCounters<Pheet>::MergeStrategyHeapPerformanceCounters(MergeStrategyHeapPerformanceCounters&)
{

}

template <class Pheet>
inline MergeStrategyHeapPerformanceCounters<Pheet>::~MergeStrategyHeapPerformanceCounters() {

}

template <class Pheet>
void MergeStrategyHeapPerformanceCounters<Pheet>::print_headers() {
}

template <class Pheet>
void MergeStrategyHeapPerformanceCounters<Pheet>::print_values() {

}

}

#endif /* MERGESTRATEGYHEAPPERFORMANCECOUNTERS_H_ */
