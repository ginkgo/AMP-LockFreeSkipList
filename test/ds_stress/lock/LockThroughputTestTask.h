/*
 * LockThroughputTestTask.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LOCKTHROUGHPUTTESTTASK_H_
#define LOCKTHROUGHPUTTESTTASK_H_

#include <pheet/primitives/Reducer/Sum/SumReducer.h>
#include <pheet/primitives/Reducer/Min/MinReducer.h>
#include <pheet/primitives/Reducer/Max/MaxReducer.h>
#include <random>
#include <functional>
#include <chrono>
#include <atomic>

namespace pheet {

template <class Pheet, class Ds>
class LockThroughputTestTask : public Pheet::Task {
public:
	typedef LockThroughputTestTask<Pheet, Ds> Self;

	typedef typename Ds::LockGuard LockGuard;

	LockThroughputTestTask(Ds* locks, size_t num_locks, int pattern, size_t instances,
			SumReducer<Pheet, size_t>& throughput,
			MinReducer<Pheet, size_t>& throughput_min,
			MaxReducer<Pheet, size_t>& throughput_max,
			std::atomic<int>& phase, std::atomic<procs_t>& ready,
			std::chrono::high_resolution_clock::time_point& start_time,
			unsigned int seed)
	: locks(locks), num_locks(num_locks), pattern(pattern), instances(instances),
	  tp(0),
	  throughput(throughput),
	  throughput_min(throughput_min),
	  throughput_max(throughput_max),
	  phase(phase), ready(ready),
	  start_time(start_time),
	  rng(seed) {

	}

	~LockThroughputTestTask() {}

	virtual void operator()() {
		std::chrono::high_resolution_clock::time_point st;
		std::uniform_int_distribution<unsigned int> r_int(0, std::numeric_limits<unsigned int>::max());
//		std::uniform_int_distribution<unsigned int> r_perc(0, 99);

		while(instances > 1) {
			size_t half = instances >> 1;
			Pheet::template
				spawn<Self>(locks, num_locks, pattern, instances - half,
						throughput,
						throughput_min,
						throughput_max,
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

		while(phase.load(std::memory_order_relaxed) == 1) {
			switch(pattern) {
			case 0:
				lock_1();
				break;
			case 1:
				lock_rec_limit(num_locks);
				break;
			case 2:
				lock_rec_limit_high_con(num_locks);
			}

			auto stop_time = std::chrono::high_resolution_clock::now();
			if(std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count() >= 1) {
				phase.store(2, std::memory_order_relaxed);
				break;
			}
		}

		throughput.add(tp);
		throughput_min.add(tp);
		throughput_max.add(tp);
	}

private:
	void lock_1() {
		std::uniform_int_distribution<size_t> r_pos(0, num_locks - 1);
		size_t pos = r_pos(rng);

		{LockGuard lg(locks[pos]);
			++tp;
		}
	}

	void lock_rec_limit(size_t limit) {
		std::uniform_int_distribution<size_t> r_pos(0, num_locks - 1);
		size_t pos = r_pos(rng);

		if(pos < limit) {
			LockGuard lg(locks[pos]);
			++tp;

			lock_rec_limit(pos);
		}
	}

	void lock_rec_limit_high_con(size_t limit) {
		std::uniform_int_distribution<size_t> r_pos(0, limit);
		size_t pos = r_pos(rng);

		if(pos < limit) {
			LockGuard lg(locks[pos]);
			++tp;

			lock_rec_limit_high_con(pos);
		}
	}

	Ds* locks;
	size_t num_locks;
	int pattern;
	procs_t instances;
	size_t tp;
	SumReducer<Pheet, size_t> throughput;
	MinReducer<Pheet, size_t> throughput_min;
	MaxReducer<Pheet, size_t> throughput_max;
	std::atomic<int>& phase;
	std::atomic<procs_t>& ready;
	std::chrono::high_resolution_clock::time_point& start_time;
	std::mt19937 rng;

};

} /* namespace pheet */
#endif /* LOCKTHROUGHPUTTESTTASK_H_ */
