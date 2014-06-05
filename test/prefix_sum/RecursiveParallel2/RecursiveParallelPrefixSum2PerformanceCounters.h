/*
 * RecursiveParallelPrefixSum2PerformanceCounters.h
 *
 *  Created on: Mar 7, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUM2PERFORMANCECOUNTERS_H_
#define RECURSIVEPARALLELPREFIXSUM2PERFORMANCECOUNTERS_H_

//#include <pheet/primitives/PerformanceCounter/List/ListPerformanceCounter.h>
//#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <iostream>

namespace pheet {

template <class Pheet>
class RecursiveParallelPrefixSum2PerformanceCounters {
public:
	typedef RecursiveParallelPrefixSum2PerformanceCounters<Pheet> Self;

	RecursiveParallelPrefixSum2PerformanceCounters() {}
	RecursiveParallelPrefixSum2PerformanceCounters(Self&)
	{}
	RecursiveParallelPrefixSum2PerformanceCounters(Self&&)
	{}
	~RecursiveParallelPrefixSum2PerformanceCounters() {}

	static void print_headers() {
	}
	void print_values() {

	}

};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUM2PERFORMANCECOUNTERS_H_ */
