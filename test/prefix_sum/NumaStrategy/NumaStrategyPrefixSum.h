/*
 * NumaStrategyPrefixSum.h
 *
 *  Created on: Mar 07, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef NUMASTRATEGYPREFIXSUM_H_
#define NUMASTRATEGYPREFIXSUM_H_

#include <pheet/pheet.h>
#include <pheet/misc/align.h>
#include "NumaStrategyPrefixSumOffsetTask.h"
#include "NumaStrategyPrefixSumLocalSumTask.h"
#include "NumaStrategyPrefixSumPerformanceCounters.h"

#include <iostream>
#include <atomic>

namespace pheet {

/*
 * Due to lack of OS support this benchmark cannot be used for now
 */
template <class Pheet, size_t BlockSize, bool Inclusive>
class NumaStrategyPrefixSumImpl : public Pheet::Task {
public:
	typedef NumaStrategyPrefixSumImpl<Pheet, BlockSize, Inclusive> Self;
	typedef NumaStrategyPrefixSumImpl<Pheet, BlockSize, false> ExclusiveSelf;
	typedef NumaStrategyPrefixSumOffsetTask<Pheet, BlockSize, Inclusive> OffsetTask;
	typedef NumaStrategyPrefixSumLocalSumTask<Pheet, BlockSize, Inclusive> LocalSumTask;
	typedef NumaStrategyPrefixSumPerformanceCounters<Pheet> PerformanceCounters;

	typedef typename Pheet::Place Place;

	NumaStrategyPrefixSumImpl(unsigned int* data, size_t length, Place* locality_data)
	:data(data), length(length) {

	}

	NumaStrategyPrefixSumImpl(unsigned int* data, size_t length, Place* locality_data, PerformanceCounters& pc)
	:data(data), length(length), pc(pc) {

	}

	virtual ~NumaStrategyPrefixSumImpl() {}

	virtual void operator()() {
		if(length <= BlockSize) {
			unsigned int sum = 0;
			for(size_t i = 0; i < length; ++i) {
				unsigned int tmp = data[i];
				if(Inclusive) {
					sum += tmp;
					data[i] = sum;
				}
				else {
					data[i] = sum;
					sum += tmp;
				}
			}
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;

			aligned_data<unsigned int, 64> auxiliary_data(num_blocks);
			aligned_data<procs_t, 64> numa_nodes(num_blocks);

			std::atomic<size_t> sequential(0);

			pc.numa_cache_time.start_timer();
			// Less overhead if initialized sequentially...
			for(size_t i = 0; i < num_blocks; ++i) {
				numa_nodes.ptr()[i] = Pheet::get_place()->get_data_numa_node_id(data + BlockSize * i);
			}
			pc.numa_cache_time.stop_timer();

			// Calculate offsets
			Pheet::template finish<OffsetTask>(data, auxiliary_data.ptr(), numa_nodes.ptr(), num_blocks, length, 0, sequential, Pheet::get_place());
			size_t seq = sequential.load(std::memory_order_relaxed);
			pc.blocks.add(num_blocks);
			pc.preprocessed_blocks.add(seq);
			if(seq < num_blocks) {
				// Prefix sum on offsets
				pheet_assert(seq > 0);
				Pheet::template finish<ExclusiveSelf>(auxiliary_data.ptr() + seq - 1, num_blocks + 1 - seq, pc);

				// Calculate local prefix sums based on offset
				Pheet::template finish<LocalSumTask>(data + (seq * BlockSize), auxiliary_data.ptr() + seq, numa_nodes.ptr() + seq, num_blocks - seq, length - (seq * BlockSize), seq, pc);
			}
		}
	}

	static char const name[];

private:
	unsigned int* data;
	size_t length;
	PerformanceCounters pc;
};

template <class Pheet, size_t BlockSize, bool Inclusive>
char const NumaStrategyPrefixSumImpl<Pheet, BlockSize, Inclusive>::name[] = "NumaStrategyPrefixSum";

template <class Pheet>
using NumaStrategyPrefixSum = NumaStrategyPrefixSumImpl<Pheet, 4096, true>;

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUM_H_ */
