/*
 * SetDsInitTask.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SETDSINITTASK_H_
#define SETDSINITTASK_H_

#include <random>
#include <functional>

namespace pheet {

template <class Pheet, class Ds>
class SetDsInitTask : public Pheet::Task {
public:
	typedef SetDsInitTask<Pheet, Ds> Self;

	SetDsInitTask(Ds& ds, size_t size, unsigned int max_v, unsigned int seed)
	: ds(ds), size(size), max_v(max_v), rng(seed) {

	}
	~SetDsInitTask() {

	}

	virtual void operator()() {
		std::uniform_int_distribution<unsigned int> r_int(0, std::numeric_limits<unsigned int>::max());
		std::uniform_int_distribution<unsigned int> r_v(0, max_v);

		while(size > 256) {
			size_t half = size >> 1;
			Pheet::template
				spawn<Self>(ds, size - half, max_v, r_int(rng));
			size = half;
		}

		for(size_t i = 0; i < size; ++i) {
			while(!ds.add(r_v(rng)));
		}
	}

private:
	Ds& ds;
	size_t size;
	unsigned int max_v;
	std::mt19937 rng;
};

} /* namespace pheet */
#endif /* SETDSINITTASK_H_ */
