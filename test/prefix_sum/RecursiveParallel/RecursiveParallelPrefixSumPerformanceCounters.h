/*
 * RecursiveParallelPrefixSumPerformanceCounters.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUMPERFORMANCECOUNTERS_H_
#define RECURSIVEPARALLELPREFIXSUMPERFORMANCECOUNTERS_H_

//#include <pheet/primitives/PerformanceCounter/List/ListPerformanceCounter.h>
//#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <iostream>

namespace pheet {

template <class Pheet>
class RecursiveParallelPrefixSumPerformanceCounters {
public:
	typedef RecursiveParallelPrefixSumPerformanceCounters<Pheet> Self;

	RecursiveParallelPrefixSumPerformanceCounters() {}
	RecursiveParallelPrefixSumPerformanceCounters(Self&)
	{}
	RecursiveParallelPrefixSumPerformanceCounters(Self&&)
	{}
	~RecursiveParallelPrefixSumPerformanceCounters() {}

	static void print_headers() {
	}
	void print_values() {

	}

};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUMPERFORMANCECOUNTERS_H_ */
