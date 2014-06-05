/*
 * CountBenchHyperTask.h
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef COUNTBENCHHYPERTASK_H_
#define COUNTBENCHHYPERTASK_H_

#include <random>
#include "../init.h"

namespace pheet {

template <class Pheet, class Hyper>
class CountBenchHyperTask : public Pheet::Task {
public:
	typedef CountBenchHyperTask<Pheet, Hyper> Self;

	CountBenchHyperTask(Hyper& count, unsigned int seed, size_t blocks, double p)
	:count(count), seed(seed), blocks(blocks), p(p) {}
	~CountBenchHyperTask() {

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
	Hyper count;
	unsigned int seed;
	size_t blocks;
	double p;
};

} /* namespace pheet */
#endif /* COUNTBENCHHYPERTASK_H_ */
