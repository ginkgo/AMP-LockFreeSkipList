/*
 * RecursiveParallelPrefixSumVectorized2.h
 *
 *  Created on: Mar 23, 2014
 *      Author: Martin Wimmer, Manuel Pöter
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUMVECTORIZED2_H_
#define RECURSIVEPARALLELPREFIXSUMVECTORIZED2_H_

#include <iostream>
#include <pheet/pheet.h>
#include <pheet/misc/align.h>
#include "RecursiveParallelPrefixSumVectorized2InstructionSet.h"
#include "RecursiveParallelPrefixSumVectorized2OffsetTask.h"
#include "RecursiveParallelPrefixSumVectorized2LocalSumTask.h"
#include "RecursiveParallelPrefixSumVectorized2PerformanceCounters.h"

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive, VectorizationInstructionSet InstructionSet>
class RecursiveParallelPrefixSumVectorizedImpl2 : public Pheet::Task {
public:
	typedef RecursiveParallelPrefixSumVectorizedImpl2<Pheet, BlockSize, Inclusive, InstructionSet> Self;
	typedef RecursiveParallelPrefixSumVectorizedImpl2<Pheet, BlockSize, false, InstructionSet> ExclusiveSelf;
	typedef RecursiveParallelPrefixSumVectorized2OffsetTask<Pheet, BlockSize, InstructionSet> OffsetTask;
	typedef RecursiveParallelPrefixSumVectorized2LocalSumTask<Pheet, BlockSize, Inclusive, InstructionSet> LocalSumTask;
	typedef RecursiveParallelPrefixSumVectorized2PerformanceCounters<Pheet> PerformanceCounters;

	RecursiveParallelPrefixSumVectorizedImpl2(unsigned int* data, size_t length)
	:data(data), length(length), step(1), root(true) {

	}

	RecursiveParallelPrefixSumVectorizedImpl2(unsigned int* data, size_t length, size_t step, bool root)
	:data(data), length(length), step(step), root(root) {

	}

	RecursiveParallelPrefixSumVectorizedImpl2(unsigned int* data, size_t length, PerformanceCounters& pc)
	:data(data), length(length), step(1), root(true), pc(pc) {

	}

	RecursiveParallelPrefixSumVectorizedImpl2(unsigned int* data, size_t length, size_t step, bool root, PerformanceCounters& pc)
	:data(data), length(length), step(step), root(root), pc(pc) {

	}

	virtual ~RecursiveParallelPrefixSumVectorizedImpl2() {}

	virtual void operator()()
	{
		if (length <= BlockSize)
		{
			if (Inclusive)
				PrefixSumCalculator<InstructionSet>::calculateInclusivePrefixSum(data, length, 0);
			else
			{
				PrefixSumCalculator<InstructionSet>::calculateExclusivePrefixSum(data, length, 0);
			}
		}
		else
		{
			size_t num_blocks = ((length - 1) / BlockSize) + 1;

			aligned_data<unsigned int, 64> auxiliary_data(num_blocks);

			// Calculate offsets
			Pheet::template finish<OffsetTask>(data, auxiliary_data.ptr(), num_blocks, length);
			// Prefix sum on offsets
			Pheet::template finish<ExclusiveSelf>(auxiliary_data.ptr(), num_blocks, pc);
			// Calculate local prefix sums based on offset
			Pheet::template finish<LocalSumTask>(data, auxiliary_data.ptr(), num_blocks, length);

		}
	}

	static const char* name;

private:
	unsigned int* data;
	size_t length;
	size_t step;
	bool root;
	PerformanceCounters pc;
};

template <VectorizationInstructionSet InstructionSet>
struct ImplName
{
	static const char* name;
};

template <>
const char* ImplName<VectorizationInstructionSet::SSE2>::name = "RecursiveParallelPrefixSumVectorized2SSE2";
template <>
const char* ImplName<VectorizationInstructionSet::SSE4>::name = "RecursiveParallelPrefixSumVectorized2SSE4";
template <>
const char* ImplName<VectorizationInstructionSet::AVX2>::name = "RecursiveParallelPrefixSumVectorized2AVX2";

template <class Pheet, size_t BlockSize, bool Inclusive, VectorizationInstructionSet InstructionSet>
const char* RecursiveParallelPrefixSumVectorizedImpl2<Pheet, BlockSize, Inclusive, InstructionSet>::name = ImplName<InstructionSet>::name;

#ifdef __MIC__
template <class Pheet>
using RecursiveParallelPrefixSumVectorized2MIC = RecursiveParallelPrefixSumVectorizedImpl2<Pheet, 4096, true, VectorizationInstructionSet::MIC>;
#else
template <class Pheet>
using RecursiveParallelPrefixSumVectorized2SSE2 = RecursiveParallelPrefixSumVectorizedImpl2<Pheet, 4096, true, VectorizationInstructionSet::SSE2>;
template <class Pheet>
using RecursiveParallelPrefixSumVectorized2SSE4 = RecursiveParallelPrefixSumVectorizedImpl2<Pheet, 4096, true, VectorizationInstructionSet::SSE4>;
template <class Pheet>
using RecursiveParallelPrefixSumVectorized2AVX2 = RecursiveParallelPrefixSumVectorizedImpl2<Pheet, 4096, true, VectorizationInstructionSet::AVX2>;
#endif
} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUMVECTORIZED2_H_ */
