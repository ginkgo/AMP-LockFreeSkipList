/*
 * ModularTaskStoragePerformanceCounters.h
 *
 *  Created on: Oct 28, 2011
 *      Author: Martin Wimmer
 */

#ifndef MODULARTASKSTORAGEPERFORMANCECOUNTERS_H_
#define MODULARTASKSTORAGEPERFORMANCECOUNTERS_H_

#include "../../../settings.h"

#include "../../../primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "../../../primitives/PerformanceCounter/Time/TimePerformanceCounter.h"

namespace pheet {

template <class Scheduler, class PrimaryPerformanceCounters, class SecondaryPerformanceCounters>
class ModularTaskStoragePerformanceCounters {
public:
	ModularTaskStoragePerformanceCounters();
	ModularTaskStoragePerformanceCounters(ModularTaskStoragePerformanceCounters& other);
	~ModularTaskStoragePerformanceCounters();

	static void print_headers();
	void print_values();

//private:
	PrimaryPerformanceCounters primary_perf_count;
	SecondaryPerformanceCounters secondary_perf_count;
};

template <class Scheduler, class PrimaryPerformanceCounters, class SecondaryPerformanceCounters>
inline ModularTaskStoragePerformanceCounters<Scheduler, PrimaryPerformanceCounters, SecondaryPerformanceCounters>::ModularTaskStoragePerformanceCounters() {

}

template <class Scheduler, class PrimaryPerformanceCounters, class SecondaryPerformanceCounters>
inline ModularTaskStoragePerformanceCounters<Scheduler, PrimaryPerformanceCounters, SecondaryPerformanceCounters>::ModularTaskStoragePerformanceCounters(ModularTaskStoragePerformanceCounters& other)
:primary_perf_count(other.primary_perf_count),
 secondary_perf_count(other.secondary_perf_count)
{

}

template <class Scheduler, class PrimaryPerformanceCounters, class SecondaryPerformanceCounters>
inline ModularTaskStoragePerformanceCounters<Scheduler, PrimaryPerformanceCounters, SecondaryPerformanceCounters>::~ModularTaskStoragePerformanceCounters() {
}

template <class Scheduler, class PrimaryPerformanceCounters, class SecondaryPerformanceCounters>
inline void ModularTaskStoragePerformanceCounters<Scheduler, PrimaryPerformanceCounters, SecondaryPerformanceCounters>::print_headers() {
	PrimaryPerformanceCounters::print_headers();
	SecondaryPerformanceCounters::print_headers();
}

template <class Scheduler, class PrimaryPerformanceCounters, class SecondaryPerformanceCounters>
inline void ModularTaskStoragePerformanceCounters<Scheduler, PrimaryPerformanceCounters, SecondaryPerformanceCounters>::print_values() {
	primary_perf_count.print_values();
	secondary_perf_count.print_values();
}

}

#endif /* MODULARTASKSTORAGEPERFORMANCECOUNTERS_H_ */
