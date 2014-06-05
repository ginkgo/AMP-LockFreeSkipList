/*
 * RecursiveParallelPrefixSumVectorized2LocalSumTask.h
 *
 *  Created on: 23.03.2014
 *      Author: Martin Wimmer, Manuel Pöter
 *     License: Pheet License
 */

#ifndef RECURSIVEPARALLELPREFIXSUMVECTORIZED2LOCALSUMTASK_H_
#define RECURSIVEPARALLELPREFIXSUMVECTORIZED2LOCALSUMTASK_H_

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive, VectorizationInstructionSet InstructionSet>
class RecursiveParallelPrefixSumVectorized2LocalSumTask : public Pheet::Task {
public:
	typedef RecursiveParallelPrefixSumVectorized2LocalSumTask<Pheet, BlockSize, Inclusive, InstructionSet> Self;

	RecursiveParallelPrefixSumVectorized2LocalSumTask(unsigned int* data, unsigned int* auxiliary_data, size_t blocks, size_t length)
		:data(data), auxiliary_data(auxiliary_data), blocks(blocks), length(length) {}
	~RecursiveParallelPrefixSumVectorized2LocalSumTask() {}

	virtual void operator()()
	{
		if (blocks == 1)
		{
			if (Inclusive)
				PrefixSumCalculator<InstructionSet>::calculateInclusivePrefixSum(data, length, auxiliary_data[0]);
			else
				PrefixSumCalculator<InstructionSet>::calculateExclusivePrefixSum(data, length, auxiliary_data[0]);
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
#endif /* RECURSIVEPARALLELPREFIXSUMVECTORIZED2LOCALSUMTASK_H_ */
