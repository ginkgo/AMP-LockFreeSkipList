/*
 * SynchroneousSchedulerPerformanceCounters.h
 *
 *  Created on: 12.02.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SYNCHRONEOUSSCHEDULERPERFORMANCECOUNTERS_H_
#define SYNCHRONEOUSSCHEDULERPERFORMANCECOUNTERS_H_

#include "../../settings.h"
/*
#include "../../../primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "../../../primitives/PerformanceCounter/Max/MaxPerformanceCounter.h"
#include "../../../primitives/PerformanceCounter/Min/MinPerformanceCounter.h"
#include "../../../primitives/PerformanceCounter/Time/TimePerformanceCounter.h"
*/
namespace pheet {

template <class Pheet>
class SynchroneousSchedulerPerformanceCounters {
public:
	SynchroneousSchedulerPerformanceCounters() {}
	SynchroneousSchedulerPerformanceCounters(SynchroneousSchedulerPerformanceCounters<Pheet>&)
		/*: num_calls(other.num_calls)*/ {}

	static void print_headers();
	void print_values();

//private:
//	BasicPerformanceCounter<Pheet, scheduler_count_calls> num_calls;
};

template <class Pheet>
inline void SynchroneousSchedulerPerformanceCounters<Pheet>::print_headers() {
//	BasicPerformanceCounter<Pheet, scheduler_count_calls>::print_header("calls\t");
}

template <class Pheet>
inline void SynchroneousSchedulerPerformanceCounters<Pheet>::print_values() {
//	num_calls.print("%lu\t");
}

}

#endif /* SYNCHRONEOUSSCHEDULERPERFORMANCECOUNTERS_H_ */
