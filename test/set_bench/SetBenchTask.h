/*
 * SetBenchTask.h
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef SETBENCHTASK_H_
#define SETBENCHTASK_H_

#include <random>
#include "../init.h"

namespace pheet {

template <class Pheet, class Set>
class SetBenchTask : public Pheet::Task {
public:
	typedef SetBenchTask<Pheet, Set> Self;

	SetBenchTask(Set& set, size_t range, unsigned int seed, size_t blocks, double contains_p, double add_p)
	:set(set), range(range), seed(seed), blocks(blocks), contains_p(contains_p), add_p(add_p) {}
	~SetBenchTask() {

	}

	virtual void operator()() {
		while(blocks > 1) {
			size_t half = blocks >> 1;
			Pheet::template
				spawn<Self>(set, range, seed + half, blocks - half, contains_p, add_p);
			blocks = half;
		}

		std::mt19937 rng(seed);
		std::uniform_int_distribution<size_t> rnd(0, range);
		std::uniform_real_distribution<double> dis(0, 1);

		for(size_t i = 0; i <= 16384; ++i) {
			size_t item = rnd(rng);
			double op = dis(rng);
			if(op < contains_p) {
				set.contains(item);
			}
			else if(op < contains_p + add_p) {
				set.put(item);
			}
			else {
				set.remove(item);
			}
		}
	}

private:
	Set& set;
	size_t range;
	unsigned int seed;
	size_t blocks;
	double contains_p;
	double add_p;
};

} /* namespace pheet */
#endif /* SETBENCHTASK_H_ */
