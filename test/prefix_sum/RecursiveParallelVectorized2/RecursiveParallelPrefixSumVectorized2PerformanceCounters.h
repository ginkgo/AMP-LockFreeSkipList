/*
 * RecursiveParallelPrefixSumVectorized2PerformanceCounters.h
 *
 *  Created on: Mar 23, 2014
 *      Author: Martin Wimmer, Manuel Pöter
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUMVECTORIZED2PERFORMANCECOUNTERS_H_
#define RECURSIVEPARALLELPREFIXSUMVECTORIZED2PERFORMANCECOUNTERS_H_

//#include <pheet/primitives/PerformanceCounter/List/ListPerformanceCounter.h>
//#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <iostream>

namespace pheet {

template <class Pheet>
class RecursiveParallelPrefixSumVectorized2PerformanceCounters {
public:
	typedef RecursiveParallelPrefixSumVectorized2PerformanceCounters<Pheet> Self;

	RecursiveParallelPrefixSumVectorized2PerformanceCounters() {}
	RecursiveParallelPrefixSumVectorized2PerformanceCounters(Self&)
	{}
	RecursiveParallelPrefixSumVectorized2PerformanceCounters(Self&&)
	{}
	~RecursiveParallelPrefixSumVectorized2PerformanceCounters() {}

	static void print_headers() {
	}
	void print_values() {

	}

};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUMVECTORIZED2PERFORMANCECOUNTERS_H_ */
