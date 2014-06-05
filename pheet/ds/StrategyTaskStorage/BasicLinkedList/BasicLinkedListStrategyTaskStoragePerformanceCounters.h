/*
 * BasicLinkedListStrategyTaskStoragePerformanceCounters.h
 *
 *  Created on: 06.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICLINKEDLISTSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_H_
#define BASICLINKEDLISTSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_H_

namespace pheet {

template <class Pheet, class StrategyHeapPerformanceCounters>
class BasicLinkedListStrategyTaskStoragePerformanceCounters {
public:
	typedef BasicLinkedListStrategyTaskStoragePerformanceCounters<Pheet, StrategyHeapPerformanceCounters> Self;

	inline BasicLinkedListStrategyTaskStoragePerformanceCounters() {}
	inline BasicLinkedListStrategyTaskStoragePerformanceCounters(Self& other)
	: strategy_heap_performance_counters(other.strategy_heap_performance_counters) {}

	inline ~BasicLinkedListStrategyTaskStoragePerformanceCounters() {}

	inline static void print_headers() {
		StrategyHeapPerformanceCounters::print_headers();
	}
	inline void print_values() {
		strategy_heap_performance_counters.print_values();
	}

	StrategyHeapPerformanceCounters strategy_heap_performance_counters;
};

}

#endif /* BASICLINKEDLISTSTRATEGYTASKSTORAGEPERFORMANCECOUNTERS_H_ */
