/*
 * CountBenchTask.h
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef COUNTBENCHTASK_H_
#define COUNTBENCHTASK_H_

#include <random>
#include "../init.h"

namespace pheet {

template <class Pheet, class Count>
class CountBenchTask : public Pheet::Task {
public:
	typedef CountBenchTask<Pheet, Count> Self;

	CountBenchTask(Count& count, unsigned int seed, size_t blocks, double p)
	:count(count), seed(seed), blocks(blocks), p(p) {}
	~CountBenchTask() {

	}

	virtual void operator()() {
		while(blocks > 1) {
			size_t half = blocks >> 1;
			Pheet::template
				spawn<Self>(count, seed + half, blocks - half, p);
			blocks = half;
		}

		std::mt19937 rng(seed);
		std::uniform_real_distribution<double> dis(0, 1);

		for(size_t i = 0; i <= 16384; ++i) {
			double op = dis(rng);
			if(op < p) {
				count.incr();
			}
		}
	}

private:
	Count& count;
	unsigned int seed;
	size_t blocks;
	double p;
};

} /* namespace pheet */
#endif /* COUNTBENCHTASK_H_ */
