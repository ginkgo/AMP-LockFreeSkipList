/*
 * StrategyRecursiveParallelPrefixSum.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGYRECURSIVEPARALLELPREFIXSUM_H_
#define STRATEGYRECURSIVEPARALLELPREFIXSUM_H_

#include <iostream>
#include <pheet/pheet.h>
#include "StrategyRecursiveParallelPrefixSumOffsetTask.h"
#include "StrategyRecursiveParallelPrefixSumStrategy.h"
#include "StrategyRecursiveParallelPrefixSumPerformanceCounters.h"

namespace pheet {

template <class Pheet, size_t BlockSize>
class StrategyRecursiveParallelPrefixSumTask : public Pheet::Task {
public:
	typedef StrategyRecursiveParallelPrefixSumTask<Pheet, BlockSize> Self;
	typedef StrategyRecursiveParallelPrefixSumStrategy<Pheet> Strategy;
	typedef StrategyRecursiveParallelPrefixSumPerformanceCounters<Pheet> PerformanceCounters;
	typedef StrategyRecursiveParallelPrefixSumEvent<Pheet> Event;

	StrategyRecursiveParallelPrefixSumTask(unsigned int* data, size_t length, size_t step, size_t ordering, size_t& preprocessed)
	:data(data), length(length), step(step), ordering(ordering), preprocessed(preprocessed) {

	}

	StrategyRecursiveParallelPrefixSumTask(unsigned int* data, size_t length, size_t step, size_t ordering, size_t& preprocessed, PerformanceCounters& pc)
	:data(data), length(length), step(step), ordering(ordering), preprocessed(preprocessed), pc(pc) {

	}

	virtual ~StrategyRecursiveParallelPrefixSumTask() {}

	virtual void operator()() {
		pc.schedule.add(Event(Pheet::get_place_id(), length, ordering, preprocessed));
		bool in_order = (ordering == preprocessed);
		pheet_assert(preprocessed <= ordering);
		if(length <= BlockSize) {
			pc.prefix_sum_blocks.incr();
			if(in_order && ordering != 0) {
				data[0] += *(data - step);
			}
			for(size_t i = 1; i < length; ++i) {
				data[i*step] += data[(i - 1)*step];
			}
			if(in_order) {
				MEMORY_FENCE();//sleep(10000);
				pc.prefix_sum_preprocessed_blocks.incr();
				++preprocessed;
			}
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;
			size_t half_blocks = (num_blocks >> 1);
			size_t half = BlockSize * half_blocks;

			Pheet::template
				spawn_s<Self>(
						Strategy(data, half, in_order, false),
						data, half, step, ordering, preprocessed, pc);
			Pheet::template
				spawn_s<Self>(
						Strategy(data + half*step, length - half, in_order, (ordering + half_blocks) != preprocessed),
						data + half*step, length - half, step, ordering + half_blocks, preprocessed, pc);
		}
	}

	static char const name[];

private:
	unsigned int* data;
	size_t length;
	size_t step;
	size_t ordering;
	size_t& preprocessed;

	PerformanceCounters pc;
};

template <class Pheet, size_t BlockSize>
class StrategyRecursiveParallelPrefixSumImpl : public Pheet::Task {
public:
	typedef StrategyRecursiveParallelPrefixSumImpl<Pheet, BlockSize> Self;
	typedef StrategyRecursiveParallelPrefixSumTask<Pheet, BlockSize> Task;
	typedef StrategyRecursiveParallelPrefixSumOffsetTask<Pheet, BlockSize> OffsetTask;
	typedef StrategyRecursiveParallelPrefixSumStrategy<Pheet> Strategy;
	typedef StrategyRecursiveParallelPrefixSumPerformanceCounters<Pheet> PerformanceCounters;
	typedef StrategyRecursiveParallelPrefixSumEvent<Pheet> Event;

	StrategyRecursiveParallelPrefixSumImpl(unsigned int* data, size_t length)
	:data(data), length(length), step(1) {

	}

	StrategyRecursiveParallelPrefixSumImpl(unsigned int* data, size_t length, size_t step)
	:data(data), length(length), step(step) {

	}

	StrategyRecursiveParallelPrefixSumImpl(unsigned int* data, size_t length, PerformanceCounters& pc)
	:data(data), length(length), step(1), pc(pc) {

	}

	StrategyRecursiveParallelPrefixSumImpl(unsigned int* data, size_t length, size_t step, PerformanceCounters& pc)
	:data(data), length(length), step(step), pc(pc) {

	}

	virtual ~StrategyRecursiveParallelPrefixSumImpl() {}

	virtual void operator()() {
		pc.schedule.add(Event(Pheet::get_place_id(), length, 0, 0));
		if(length <= BlockSize) {
			pc.prefix_sum_blocks.incr();
			for(size_t i = 1; i < length; ++i) {
				data[i*step] += data[(i - 1)*step];
			}
			pc.prefix_sum_preprocessed_blocks.incr();
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;
			size_t half_blocks = (num_blocks >> 1);
			size_t half = BlockSize * half_blocks;

			size_t pp = 0;
			{typename Pheet::Finish f;
				Pheet::template
					spawn_s<Task>(
							Strategy(data, half, true, false),
							data, half, step, 0, pp, pc);
				Pheet::template
					spawn_s<Task>(
							Strategy(data + half*step, length - half, true, half_blocks != pp),
							data + half*step, length - half, step, half_blocks, pp, pc);
			}
//std::cout << data[1024] << std::endl;
			if(pp != num_blocks) {
	//			std::cout << "pp: " << pp << " nb: " << num_blocks << std::endl;
				size_t valid = pp*BlockSize;
				if((num_blocks - pp) > 1)
					Pheet::template
						finish<Self>(data + (valid - 1)*step, num_blocks - pp, BlockSize * step, pc);
//				std::cout << data[1024] << std::endl;

				Pheet::template
					call<OffsetTask>(data + (valid*step), length - valid, step);
			}
		}
	}

	static char const name[];

private:
	unsigned int* data;
	size_t length;
	size_t step;
	PerformanceCounters pc;
};

template <class Pheet, size_t BlockSize>
char const StrategyRecursiveParallelPrefixSumImpl<Pheet, BlockSize>::name[] = "StrategyRecursiveParallelPrefixSum";

template <class Pheet>
using StrategyRecursiveParallelPrefixSum = StrategyRecursiveParallelPrefixSumImpl<Pheet, 4096>;

} /* namespace pheet */
#endif /* STRATEGYRECURSIVEPARALLELPREFIXSUM_H_ */
