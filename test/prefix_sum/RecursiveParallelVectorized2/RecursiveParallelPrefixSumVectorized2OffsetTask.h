/*
 * RecursiveParallelPrefixSumVectorized2OffsetTask.h
 *
 *  Created on: Mar 23, 2014
 *      Author: Martin Wimmer, Manuel Pöter
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUMVECTORIZED2OFFSETTASK_H_
#define RECURSIVEPARALLELPREFIXSUMVECTORIZED2OFFSETTASK_H_

#include <pheet/pheet.h>

namespace pheet {

template <class Pheet, size_t BlockSize, VectorizationInstructionSet InstructionSet>
class RecursiveParallelPrefixSumVectorized2OffsetTask : public Pheet::Task {
public:
	typedef RecursiveParallelPrefixSumVectorized2OffsetTask<Pheet, BlockSize, InstructionSet> Self;

	RecursiveParallelPrefixSumVectorized2OffsetTask(unsigned int* data, unsigned int* auxiliary_data, size_t blocks, size_t length)
	:data(data), auxiliary_data(auxiliary_data), blocks(blocks), length(length) {}
	virtual ~RecursiveParallelPrefixSumVectorized2OffsetTask() {}

	virtual void operator()()
	{
		if (blocks == 1)
		{
			auxiliary_data[0] = PrefixSumCalculator<InstructionSet>::calculateSum(data, length);
		}
		else
		{
			size_t half = blocks >> 1;
			size_t half_l = half * BlockSize;

			Pheet::template
				spawn<Self>(data + half_l, auxiliary_data + half, blocks - half, length - half_l);

			Pheet::template
				call<Self>(data, auxiliary_data, half, half_l);
		}
	}
private:
	unsigned int* data;
	unsigned int* auxiliary_data;
	size_t blocks;
	size_t length;
};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUMVECTORIZED2OFFSETTASK_H_ */
