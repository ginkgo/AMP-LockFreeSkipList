/*
 * DummyPerformanceCounters.h
 *
 *  Created on: Jul 31, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef DUMMYPERFORMANCECOUNTERS_H_
#define DUMMYPERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet>
class DummyPerformanceCounters {
public:
	typedef DummyPerformanceCounters<Pheet> Self;

	DummyPerformanceCounters() {}
	DummyPerformanceCounters(Self& other) {}
	~DummyPerformanceCounters() {}

	static void print_headers() {}
	void print_values() {}
};

} /* namespace pheet */
#endif /* DUMMYPERFORMANCECOUNTERS_H_ */
