/*
 * LinearDsInitTask.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LINEARDSINITTASK_H_
#define LINEARDSINITTASK_H_

#include <random>
#include <functional>

namespace pheet {

template <class Pheet, class Ds>
class LinearDsInitTask : public Pheet::Task {
public:
	typedef LinearDsInitTask<Pheet, Ds> Self;

	LinearDsInitTask(Ds& ds, size_t size, unsigned int seed)
	: ds(ds), size(size), rng(seed) {

	}
	~LinearDsInitTask() {

	}

	virtual void operator()() {
		std::uniform_int_distribution<unsigned int> r_int(0, std::numeric_limits<unsigned int>::max());

		while(size > 256) {
			size_t half = size >> 1;
			Pheet::template
				spawn<Self>(ds, size - half, r_int(rng));
			size = half;
		}

		for(size_t i = 0; i < size; ++i) {
			ds.push(r_int(rng));
		}
	}

private:
	Ds& ds;
	size_t size;
	std::mt19937 rng;
};

} /* namespace pheet */
#endif /* LINEARDSINITTASK_H_ */
