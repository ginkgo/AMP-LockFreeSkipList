/*
 * RecursiveParallelPrefixSum2LocalSumTask.h
 *
 *  Created on: 07.03.2014
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef RECURSIVEPARALLELPREFIXSUM2LOCALSUMTASK_H_
#define RECURSIVEPARALLELPREFIXSUM2LOCALSUMTASK_H_

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive>
class RecursiveParallelPrefixSum2LocalSumTask : public Pheet::Task {
public:
	typedef RecursiveParallelPrefixSum2LocalSumTask<Pheet, BlockSize, Inclusive> Self;

	RecursiveParallelPrefixSum2LocalSumTask(unsigned int* data, unsigned int* auxiliary_data, size_t blocks, size_t length)
		:data(data), auxiliary_data(auxiliary_data), blocks(blocks), length(length) {}
	~RecursiveParallelPrefixSum2LocalSumTask() {}

	virtual void operator()() {
		if(blocks == 1) {
			unsigned int sum = auxiliary_data[0];
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
#endif /* RECURSIVEPARALLELPREFIXSUM2LOCALSUMTASK_H_ */
