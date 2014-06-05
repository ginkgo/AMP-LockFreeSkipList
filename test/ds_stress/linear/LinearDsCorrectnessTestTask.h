/*
 * LinearDsCorrectnessTestTask.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LINEARDSCORRECTNESSTESTTASK_H_
#define LINEARDSCORRECTNESSTESTTASK_H_

#include <pheet/primitives/Reducer/Sum/SumReducer.h>
#include <pheet/primitives/Reducer/Min/MinReducer.h>
#include <pheet/primitives/Reducer/Max/MaxReducer.h>
#include <random>
#include <functional>
#include <chrono>
#include <atomic>

namespace pheet {

struct LinearDsCorrectnessTestEvent {
	procs_t id;
	int ds;
	int op;
	unsigned int value;
	bool failed;
};

template <class Pheet, class Ds>
class LinearDsCorrectnessTestTask : public Pheet::Task {
public:
	typedef LinearDsCorrectnessTestTask<Pheet, Ds> Self;

	LinearDsCorrectnessTestTask(Ds* ds,
			LinearDsCorrectnessTestEvent* history,
			size_t n, double pp, int dist,
			procs_t instances, procs_t spawn_instances,
			std::atomic<int>& phase, std::atomic<procs_t>& ready,
			unsigned int seed)
	: ds(ds), history(history), n(n), pp(pp), dist(dist), instances(instances), spawn_instances(spawn_instances),
	  phase(phase), ready(ready),
	  rng(seed) {

	}

	~LinearDsCorrectnessTestTask() {}

	virtual void operator()() {
		std::uniform_int_distribution<unsigned int> r_int(0, std::numeric_limits<unsigned int>::max());
		std::uniform_int_distribution<unsigned int> r_op(0, 1);
//		std::uniform_int_distribution<unsigned int> r_perc(0, 99);

		while(spawn_instances > 1) {
			size_t half = spawn_instances >> 1;
			Pheet::template
				spawn<Self>(ds,
						history,
						n, pp, dist,
						instances, spawn_instances - half,
						phase, ready,
						r_int(rng));
			spawn_instances = half;
		}

		procs_t id = ready.fetch_sub(1, std::memory_order_acq_rel);
		pheet_assert(id != 0);
		history = history + (id - 1) * n;

		if(id == 1) {
			phase.store(1, std::memory_order_release);
		} else {
			while(phase.load(std::memory_order_acquire) == 0) {}
		}

		size_t to_push = 0, to_pop = 0;
		switch(dist) {
		case 0:	// push:pop -> 50:50
			to_push = n >> 1;
			to_pop = n - to_push;
			break;
		case 1:
			if(id & 1) {
				to_push = n;
			} else {
				to_pop = n;
			}
			break;
		}

		size_t hi = 0;
		size_t pre_push = (size_t)(to_push * pp);

		for(size_t i = 0; i < pre_push; ++i) {
			unsigned int r = r_int(rng);
			ds[r & 1].push(r);
			--to_push;
			history[hi].id = id;
			history[hi].ds = r & 1;
			history[hi].op = 0;
			history[hi].value = r;
			history[hi].failed = false;
			++hi;
		}

		procs_t new_id = ready.fetch_add(1, std::memory_order_acq_rel) + 1;
		if(new_id == instances) {
			phase.store(2, std::memory_order_release);
		} else {
			while(phase.load(std::memory_order_acquire) == 1) {}
		}

		while(to_push > 0 || to_pop > 0) {
			unsigned int r = r_int(rng);

			if(to_pop == 0 || (to_push != 0 && r_op(rng) == 0)) {
				ds[r & 1].push(r);
				--to_push;
				history[hi].id = id;
				history[hi].ds = r & 1;
				history[hi].op = 0;
				history[hi].value = r;
				history[hi].failed = false;
				++hi;
			} else {
				--to_pop;

				unsigned int v = 0;
				history[hi].failed = ds[r & 1].try_pop(v);
				history[hi].id = id;
				history[hi].ds = r & 1;
				history[hi].op = 0;
				history[hi].value = v;
				++hi;
			}
		}
	}

private:
	Ds* ds;
	LinearDsCorrectnessTestEvent* history;
	size_t n;
	double pp;
	int dist;
	procs_t instances;
	procs_t spawn_instances;
	std::atomic<int>& phase;
	std::atomic<procs_t>& ready;
	std::mt19937 rng;

};

} /* namespace pheet */
#endif /* LINEARDSCORRECTNESSTESTTASK_H_ */
