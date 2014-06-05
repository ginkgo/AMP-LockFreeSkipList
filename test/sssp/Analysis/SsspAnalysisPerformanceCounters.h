/*
 * SsspAnalysisPerformanceCounters.h
 *
 *  Created on: Jan 31, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SSSPANALYSISPERFORMANCECOUNTERS_H_
#define SSSPANALYSISPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>

namespace pheet {

template <class Pheet>
class SsspAnalysisPerformanceCounters {
public:
	typedef SsspAnalysisPerformanceCounters<Pheet> Self;

	SsspAnalysisPerformanceCounters() {}
	SsspAnalysisPerformanceCounters(Self& other)
	:num_dead_tasks(other.num_dead_tasks),
	 num_actual_tasks(other.num_actual_tasks) {}
	SsspAnalysisPerformanceCounters(Self&& other)
	:num_dead_tasks(other.num_dead_tasks),
	 num_actual_tasks(other.num_actual_tasks) {}
	~SsspAnalysisPerformanceCounters() {}

	static void print_headers() {
		BasicPerformanceCounter<Pheet, sssp_count_dead_tasks>::print_header("num_dead_tasks\t");
		BasicPerformanceCounter<Pheet, sssp_count_actual_tasks>::print_header("num_actual_tasks\t");
	}
	void print_values() {
		num_dead_tasks.print("%d\t");
		num_actual_tasks.print("%d\t");
	}

	BasicPerformanceCounter<Pheet, sssp_count_dead_tasks> num_dead_tasks;
	BasicPerformanceCounter<Pheet, sssp_count_actual_tasks> num_actual_tasks;
};

} /* namespace context */
#endif /* SSSPANALYSISPERFORMANCECOUNTERS_H_ */
