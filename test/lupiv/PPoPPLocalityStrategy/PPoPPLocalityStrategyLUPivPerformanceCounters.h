/*
 * PPoPPLocalityStrategyLUPivPerformanceCounters.h
 *
 *  Created on: Aug 2, 2012
 *      Author: Martin Wimmer, Daniel Cederman
 *	   License: Boost Software License 1.0
 */

#ifndef PPOPPLOCALITYSTRATEGYLUPIVPERFORMANCECOUNTERS_H_
#define PPOPPLOCALITYSTRATEGYLUPIVPERFORMANCECOUNTERS_H_

#include <iostream>

#include "pheet/pheet.h"

#include "pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "pheet/primitives/PerformanceCounter/List/ListPerformanceCounter.h"

namespace pheet {

struct PPoPPLocalityStrategyLUPivLocalityInfo {
	bool col;
	procs_t expected;
	procs_t actual;

	PPoPPLocalityStrategyLUPivLocalityInfo(bool col, procs_t expected, procs_t actual)
	:col(col), expected(expected), actual(actual) {}

	void print() const {
		std::cout << "{" << (col?'c':'b') << "|" << expected << ":" << actual << "}";
	}
};

template <class Pheet>
class PPoPPLocalityStrategyLUPivPerformanceCounters {
public:
	typedef PPoPPLocalityStrategyLUPivPerformanceCounters<Pheet> Self;
	typedef PPoPPLocalityStrategyLUPivLocalityInfo LocalityInfo;

	PPoPPLocalityStrategyLUPivPerformanceCounters() {}
	PPoPPLocalityStrategyLUPivPerformanceCounters(Self& other)
	:locality_misses(other.locality_misses),
	 total_distance(other.total_distance),
	 total_tasks(other.total_tasks),
	 locality(other.locality){

	}
	PPoPPLocalityStrategyLUPivPerformanceCounters(Self&& other)
	:locality_misses(other.locality_misses),
	 total_distance(other.total_distance),
	 total_tasks(other.total_tasks),
	 locality(other.locality) {

	}
	~PPoPPLocalityStrategyLUPivPerformanceCounters() {}

	static void print_headers() {
		BasicPerformanceCounter<Pheet, lupiv_count_locality_misses>::print_header("locality_misses\t");
		BasicPerformanceCounter<Pheet, lupiv_count_total_distance>::print_header("total_distance\t");
		BasicPerformanceCounter<Pheet, lupiv_total_tasks>::print_header("total_tasks\t");
		ListPerformanceCounter<Pheet, LocalityInfo, lupiv_track_locality>::print_header("locality\t");
	}
	void print_values() {
		locality_misses.print("%d\t");
		total_distance.print("%d\t");
		total_tasks.print("%d\t");
		locality.print("", "\t");
	}

	BasicPerformanceCounter<Pheet, lupiv_count_locality_misses> locality_misses;
	BasicPerformanceCounter<Pheet, lupiv_count_total_distance> total_distance;
	BasicPerformanceCounter<Pheet, lupiv_total_tasks> total_tasks;
	ListPerformanceCounter<Pheet, LocalityInfo, lupiv_track_locality> locality;
};

} /* namespace pheet */
#endif /* PPOPPLOCALITYSTRATEGYLUPIVPERFORMANCECOUNTERS_H_ */
