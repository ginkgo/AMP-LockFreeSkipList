/*
 * StrategyRecursiveParallelPrefixSumPerformanceCounters.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGYRECURSIVEPARALLELPREFIXSUMPERFORMANCECOUNTERS_H_
#define STRATEGYRECURSIVEPARALLELPREFIXSUMPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/List/ListPerformanceCounter.h>
#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <iostream>

namespace pheet {

template <class Pheet>
struct StrategyRecursiveParallelPrefixSumEvent {
public:
	typedef StrategyRecursiveParallelPrefixSumEvent<Pheet> Self;

	StrategyRecursiveParallelPrefixSumEvent(procs_t place_id, size_t length, size_t ordering, size_t pp)
	:place_id(place_id), length(length), ordering(ordering), pp(pp) {}

	StrategyRecursiveParallelPrefixSumEvent(Self const& other)
	:place_id(other.place_id), length(other.length), ordering(other.ordering), pp(other.pp) {}

	void print() const {
		std::cout << place_id << "," << length <<":" << ordering << "|" << pp << ";";
	}

private:
	procs_t place_id;
	size_t length;
	size_t ordering;
	size_t pp;
};

template <class Pheet>
class StrategyRecursiveParallelPrefixSumPerformanceCounters {
public:
	typedef StrategyRecursiveParallelPrefixSumEvent<Pheet> Event;
	typedef StrategyRecursiveParallelPrefixSumPerformanceCounters<Pheet> Self;

	StrategyRecursiveParallelPrefixSumPerformanceCounters() {}
	StrategyRecursiveParallelPrefixSumPerformanceCounters(Self& other)
	:prefix_sum_blocks(other.prefix_sum_blocks), prefix_sum_preprocessed_blocks(other.prefix_sum_preprocessed_blocks), schedule(other.schedule) {}
	StrategyRecursiveParallelPrefixSumPerformanceCounters(Self&& other)
	:prefix_sum_blocks(other.prefix_sum_blocks), prefix_sum_preprocessed_blocks(other.prefix_sum_preprocessed_blocks), schedule(std::move(other.schedule)) {}
	~StrategyRecursiveParallelPrefixSumPerformanceCounters() {}

	static void print_headers() {
		BasicPerformanceCounter<Pheet, prefix_sum_log_pf_blocks>::print_header("prefix_sum_blocks\t");
		BasicPerformanceCounter<Pheet, prefix_sum_log_pf_preprocessed_blocks>::print_header("prefix_sum_preprocessed_blocks\t");
		ListPerformanceCounter<Pheet, Event, prefix_sum_log_schedule>::print_header("schedule\t");
	}
	void print_values() {
		prefix_sum_blocks.print("%lu\t");
		prefix_sum_preprocessed_blocks.print("%lu\t");
		schedule.print("", "\t");
	}

	BasicPerformanceCounter<Pheet, prefix_sum_log_pf_blocks> prefix_sum_blocks;
	BasicPerformanceCounter<Pheet, prefix_sum_log_pf_preprocessed_blocks> prefix_sum_preprocessed_blocks;
	ListPerformanceCounter<Pheet, Event, prefix_sum_log_schedule> schedule;
};

} /* namespace pheet */
#endif /* STRATEGYRECURSIVEPARALLELPREFIXSUMPERFORMANCECOUNTERS_H_ */
