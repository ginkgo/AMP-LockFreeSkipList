/*
 * LinearDsThroughputTestTask.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LINEARDSTHROUGHPUTTESTTASK_H_
#define LINEARDSTHROUGHPUTTESTTASK_H_

#include <pheet/primitives/Reducer/Sum/SumReducer.h>
#include <pheet/primitives/Reducer/Min/MinReducer.h>
#include <pheet/primitives/Reducer/Max/MaxReducer.h>
#include <random>
#include <functional>
#include <chrono>
#include <atomic>

namespace pheet {

template <class Pheet, class Ds>
class LinearDsThroughputTestTask : public Pheet::Task {
public:
	typedef LinearDsThroughputTestTask<Pheet, Ds> Self;

	LinearDsThroughputTestTask(Ds& ds, int dist, size_t instances,
			SumReducer<Pheet, size_t>& push_throughput,
			SumReducer<Pheet, size_t>& pop_throughput,
			MinReducer<Pheet, size_t>& push_throughput_min,
			MinReducer<Pheet, size_t>& pop_throughput_min,
			MaxReducer<Pheet, size_t>& push_throughput_max,
			MaxReducer<Pheet, size_t>& pop_throughput_max,
			std::atomic<int>& phase, std::atomic<procs_t>& ready,
			std::chrono::high_resolution_clock::time_point& start_time,
			unsigned int seed)
	: ds(ds), dist(dist), instances(instances), push_throughput(push_throughput), pop_throughput(pop_throughput),
	  push_throughput_min(push_throughput_min), pop_throughput_min(pop_throughput_min),
	  push_throughput_max(push_throughput_max), pop_throughput_max(pop_throughput_max),
	  phase(phase), ready(ready),
	  start_time(start_time),
	  rng(seed) {

	}

	~LinearDsThroughputTestTask() {}

	virtual void operator()() {
		std::chrono::high_resolution_clock::time_point st;
		std::uniform_int_distribution<unsigned int> r_int(0, std::numeric_limits<unsigned int>::max());
//		std::uniform_int_distribution<unsigned int> r_perc(0, 99);

		while(instances > 1) {
			size_t half = instances >> 1;
			Pheet::template
				spawn<Self>(ds, dist, instances - half,
						push_throughput, pop_throughput,
						push_throughput_min, pop_throughput_min,
						push_throughput_max, pop_throughput_max,
						phase, ready, start_time, r_int(rng));
			instances = half;
		}

		procs_t id = ready.fetch_sub(1, std::memory_order_acq_rel);
		pheet_assert(id != 0);

		if(id == 1) {
			start_time = std::chrono::high_resolution_clock::now();
			phase.store(1, std::memory_order_release);
		} else {
			while(phase.load(std::memory_order_acquire) == 0) {}
		}

		size_t push = 0, pop = 0;
		unsigned int value = 0;
		while(phase.load(std::memory_order_relaxed) == 1) {
			switch(dist) {
			case 0:	// push:pop -> 50:50
				{unsigned int r = r_int(rng);
				for(int i = 0; i < 32; ++i, r >>= 1) {
					if(r & 1) {
						ds.push(value++);
						++push;
					} else {
						unsigned int v;
						ds.try_pop(v);
						++pop;
					}
				}}
				break;
			case 1:	// push:pop -> 50:50 - dedicated
				for(int i = 0; i < 32; ++i) {
					if(id & 1) {
						ds.push(value++);
						++push;
					} else {
						unsigned int v;
						ds.try_pop(v);
						++pop;
					}
				}
				break;
			}

			auto stop_time = std::chrono::high_resolution_clock::now();
			if(std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count() >= 1) {
				phase.store(2, std::memory_order_relaxed);
				break;
			}
		}

		switch(dist) {
		case 0:
			push_throughput.add(push);
			push_throughput_min.add(push);
			push_throughput_max.add(push);
			pop_throughput.add(pop);
			pop_throughput_min.add(pop);
			pop_throughput_max.add(pop);
			break;
		case 1:
			if(id & 1) {
				push_throughput.add(push);
				push_throughput_min.add(push);
				push_throughput_max.add(push);
			} else {
				pop_throughput.add(pop);
				pop_throughput_min.add(pop);
				pop_throughput_max.add(pop);
			}
		}
	}

private:
	Ds& ds;
	int dist;
	procs_t instances;
	SumReducer<Pheet, size_t> push_throughput;
	SumReducer<Pheet, size_t> pop_throughput;
	MinReducer<Pheet, size_t> push_throughput_min;
	MinReducer<Pheet, size_t> pop_throughput_min;
	MaxReducer<Pheet, size_t> push_throughput_max;
	MaxReducer<Pheet, size_t> pop_throughput_max;
	std::atomic<int>& phase;
	std::atomic<procs_t>& ready;
	std::chrono::high_resolution_clock::time_point& start_time;
	std::mt19937 rng;

};

} /* namespace pheet */
#endif /* LINEARDSTHROUGHPUTTESTTASK_H_ */
