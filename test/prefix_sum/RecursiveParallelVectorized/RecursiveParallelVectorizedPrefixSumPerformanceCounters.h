/*
 * RecursiveParallelVectorizedPrefixSumPerformanceCounters.h
 *
 *  Created on: Nov 06, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELVECTORIZEDPREFIXSUMPERFORMANCECOUNTERS_H_
#define RECURSIVEPARALLELVECTORIZEDPREFIXSUMPERFORMANCECOUNTERS_H_

//#include <pheet/primitives/PerformanceCounter/List/ListPerformanceCounter.h>
//#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <iostream>

namespace pheet {

template <class Pheet>
class RecursiveParallelVectorizedPrefixSumPerformanceCounters {
public:
	typedef RecursiveParallelVectorizedPrefixSumPerformanceCounters<Pheet> Self;

	RecursiveParallelVectorizedPrefixSumPerformanceCounters() {}
	RecursiveParallelVectorizedPrefixSumPerformanceCounters(Self&)
	{}
	RecursiveParallelVectorizedPrefixSumPerformanceCounters(Self&&)
	{}
	~RecursiveParallelVectorizedPrefixSumPerformanceCounters() {}

	static void print_headers() {
	}
	void print_values() {

	}

};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELVECTORIZEDPREFIXSUMPERFORMANCECOUNTERS_H_ */
