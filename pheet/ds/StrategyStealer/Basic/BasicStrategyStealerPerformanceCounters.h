/*
 * BasicStrategyStealerPerformanceCounters.h
 *
 *  Created on: 06.07.2012
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef BASICSTRATEGYSTEALERPERFORMANCECOUNTERS_H_
#define BASICSTRATEGYSTEALERPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>

namespace pheet {

template <class Pheet, class StrategyHeapPerformanceCounters>
class BasicStrategyStealerPerformanceCounters {
public:
	typedef BasicStrategyStealerPerformanceCounters<Pheet, StrategyHeapPerformanceCounters> Self;

	BasicStrategyStealerPerformanceCounters();
	BasicStrategyStealerPerformanceCounters(Self& other);
	~BasicStrategyStealerPerformanceCounters();

	static void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, stealer_count_stream_tasks> num_stream_tasks;
	BasicPerformanceCounter<Pheet, stealer_count_stolen_tasks> num_stolen_tasks;

	StrategyHeapPerformanceCounters strategy_heap_performance_counters;
};


template <class Pheet, class StrategyHeapPerformanceCounters>
inline BasicStrategyStealerPerformanceCounters<Pheet, StrategyHeapPerformanceCounters>::BasicStrategyStealerPerformanceCounters() {

}

template <class Pheet, class StrategyHeapPerformanceCounters>
BasicStrategyStealerPerformanceCounters<Pheet, StrategyHeapPerformanceCounters>::BasicStrategyStealerPerformanceCounters(BasicStrategyStealerPerformanceCounters& other)
:num_stream_tasks(other.num_stream_tasks), num_stolen_tasks(other.num_stolen_tasks), strategy_heap_performance_counters(other.strategy_heap_performance_counters) {

}

template <class Pheet, class StrategyHeapPerformanceCounters>
inline BasicStrategyStealerPerformanceCounters<Pheet, StrategyHeapPerformanceCounters>::~BasicStrategyStealerPerformanceCounters() {

}

template <class Pheet, class StrategyHeapPerformanceCounters>
void BasicStrategyStealerPerformanceCounters<Pheet, StrategyHeapPerformanceCounters>::print_headers() {
	BasicPerformanceCounter<Pheet, stealer_count_stream_tasks>::print_header("num_stream_tasks\t");
	BasicPerformanceCounter<Pheet, stealer_count_stolen_tasks>::print_header("num_stolen_tasks\t");

	StrategyHeapPerformanceCounters::print_headers();
}

template <class Pheet, class StrategyHeapPerformanceCounters>
void BasicStrategyStealerPerformanceCounters<Pheet, StrategyHeapPerformanceCounters>::print_values() {
	num_stream_tasks.print("%lu\t");
	num_stolen_tasks.print("%lu\t");

	strategy_heap_performance_counters.print_values();
}

} /* namespace pheet */
#endif /* BASICSTRATEGYSTEALERPERFORMANCECOUNTERS_H_ */
