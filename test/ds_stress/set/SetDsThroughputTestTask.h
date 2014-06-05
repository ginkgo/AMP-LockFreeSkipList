/*
 * SetDsThroughputTestTask.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SETDSTHROUGHPUTTESTTASK_H_
#define SETDSTHROUGHPUTTESTTASK_H_

#include <pheet/primitives/Reducer/Sum/SumReducer.h>
#include <pheet/primitives/Reducer/Min/MinReducer.h>
#include <pheet/primitives/Reducer/Max/MaxReducer.h>
#include <random>
#include <functional>
#include <chrono>
#include <atomic>

namespace pheet {

template <class Pheet, class Ds>
class SetDsThroughputTestTask : public Pheet::Task {
public:
	typedef SetDsThroughputTestTask<Pheet, Ds> Self;

	SetDsThroughputTestTask(Ds& ds, int dist, unsigned int max_v, size_t instances,
			SumReducer<Pheet, size_t>& add_throughput,
			SumReducer<Pheet, size_t>& remove_throughput,
			SumReducer<Pheet, size_t>& contains_throughput,
			MinReducer<Pheet, size_t>& add_throughput_min,
			MinReducer<Pheet, size_t>& remove_throughput_min,
			MinReducer<Pheet, size_t>& contains_throughput_min,
			MaxReducer<Pheet, size_t>& add_throughput_max,
			MaxReducer<Pheet, size_t>& remove_throughput_max,
			MaxReducer<Pheet, size_t>& contains_throughput_max,
			std::atomic<int>& phase, std::atomic<procs_t>& ready,
                            std::chrono::high_resolution_clock::time_point& start_time,
                            unsigned int seed)
	: ds(ds), dist(dist), max_v(max_v), instances(instances), add_throughput(add_throughput), remove_throughput(remove_throughput), contains_throughput(contains_throughput),
        add_throughput_min(add_throughput_min), remove_throughput_min(remove_throughput_min), contains_throughput_min(contains_throughput_min),
        add_throughput_max(add_throughput_max), remove_throughput_max(remove_throughput_max), contains_throughput_max(contains_throughput_max),
        phase(phase), ready(ready),
        start_time(start_time),
        rng(seed) {

	}

	~SetDsThroughputTestTask() {}

	virtual void operator()() {
		std::chrono::high_resolution_clock::time_point st;
		std::uniform_int_distribution<unsigned int> r_int(0, std::numeric_limits<unsigned int>::max());
		std::uniform_int_distribution<unsigned int> r_p(0, 99);
		std::uniform_int_distribution<unsigned int> r_v(0, max_v);

		while(instances > 1) {
			size_t half = instances >> 1;
			Pheet::template
				spawn<Self>(ds, dist, max_v, instances - half,
                            add_throughput, remove_throughput, contains_throughput,
                            add_throughput_min, remove_throughput_min, contains_throughput_min,
                            add_throughput_max, remove_throughput_max, contains_throughput_max,
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

		size_t add = 0, remove = 0, contains = 0;
		while(phase.load(std::memory_order_relaxed) == 1) {
			switch(dist) {
			case 0:	// add:remove -> 1:1:2
				{unsigned int r = r_int(rng);
                    for(int i = 0; i < 16; ++i, r >>= 2) {
                        unsigned int v = r_v(rng);
                        if(r & 2) {
                            if(r & 1) {
                                ds.add(v);
                                ++add;
                            } else {
                                ds.remove(v);
                                ++remove;
                            }
                        }
                        else {
                            ds.contains(v);
                            ++contains;
                        }
                    }}
				break;
			case 1:	// add:remove -> 1:1:2 - dedicated
				{
                    for(int i = 0; i < 16; ++i) {
                        unsigned int v = r_v(rng);
                        if(id & 2) {
                            if(id & 1) {
                                ds.add(v);
                                ++add;
                            } else {
                                ds.remove(v);
                                ++remove;
                            }
                        }
                        else {
                            ds.contains(v);
                            ++contains;
                        }
                    }}
				break;
			case 2:	// add:remove -> 1:1:98
				{
                    for(int i = 0; i < 16; ++i) {
                        unsigned int v = r_v(rng);
                        unsigned int r = r_p(rng);
                        if(r == 0) {
                            ds.add(v);
                            ++add;
                        } else if(r == 1) {
                            ds.remove(v);
                            ++remove;
                        } else {
                            ds.contains(v);
                            ++contains;
                        }
                    }}
				break;
			case 3:	// add:remove -> 1:1:P-2 dedicated
				{
                    for(int i = 0; i < 16; ++i) {
                        unsigned int v = r_v(rng);
                        if(id == 0) {
                            ds.add(v);
                            ++add;
                        } else if(id == 1) {
                            ds.remove(v);
                            ++remove;
                        } else {
                            ds.contains(v);
                            ++contains;
                        }
                    }}
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
		case 2:
			add_throughput.add(add);
			add_throughput_min.add(add);
			add_throughput_max.add(add);
			remove_throughput.add(remove);
			remove_throughput_min.add(remove);
			remove_throughput_max.add(remove);
			break;
		case 1:
			if(id & 2) {
				if(id & 1) {
					add_throughput.add(add);
					add_throughput_min.add(add);
					add_throughput_max.add(add);
				} else {
					remove_throughput.add(remove);
					remove_throughput_min.add(remove);
					remove_throughput_max.add(remove);
				}
			} else {
				contains_throughput.add(contains);
				contains_throughput_min.add(contains);
				contains_throughput_max.add(contains);
			}
			break;
		case 3:
			if(id == 0) {
				add_throughput.add(add);
				add_throughput_min.add(add);
				add_throughput_max.add(add);
			} else if(id == 1) {
				remove_throughput.add(remove);
				remove_throughput_min.add(remove);
				remove_throughput_max.add(remove);
			} else {
				contains_throughput.add(contains);
				contains_throughput_min.add(contains);
				contains_throughput_max.add(contains);
			}
		}
	}

private:
	Ds& ds;
	int dist;
	unsigned int max_v;
	procs_t instances;
	SumReducer<Pheet, size_t> add_throughput;
	SumReducer<Pheet, size_t> remove_throughput;
	SumReducer<Pheet, size_t> contains_throughput;
	MinReducer<Pheet, size_t> add_throughput_min;
	MinReducer<Pheet, size_t> remove_throughput_min;
	MinReducer<Pheet, size_t> contains_throughput_min;
	MaxReducer<Pheet, size_t> add_throughput_max;
	MaxReducer<Pheet, size_t> remove_throughput_max;
	MaxReducer<Pheet, size_t> contains_throughput_max;
	std::atomic<int>& phase;
	std::atomic<procs_t>& ready;
	std::chrono::high_resolution_clock::time_point& start_time;
	std::mt19937 rng;

};

} /* namespace pheet */
#endif /* SETDSTHROUGHPUTTESTTASK_H_ */
