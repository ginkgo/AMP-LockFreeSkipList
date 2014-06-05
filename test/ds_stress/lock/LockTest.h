/*
 * LockTest.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LOCKTEST_H_
#define LOCKTEST_H_

#include <pheet/pheet.h>
#include "../../Test.h"
#include <random>
#include <chrono>

#include "LockThroughputTestTask.h"
#include <pheet/primitives/Reducer/Sum/SumReducer.h>
#include <pheet/primitives/Reducer/Min/MinReducer.h>
#include <pheet/primitives/Reducer/Max/MaxReducer.h>


namespace pheet {

template <class Pheet, template <class> class DsT>
class LockTest {
public:
	typedef DsT<Pheet> Ds;

	typedef LockThroughputTestTask<Pheet, Ds> Throughput;

	LockTest(procs_t cpus, int type, unsigned int seed)
	: cpus(cpus), type(type), seed(seed) {

	}

	~LockTest() {}

	void run_test() {
		for(size_t l = 0; l < sizeof(ds_stress_test_num_locks)/sizeof(ds_stress_test_num_locks[0]); l++) {
			size_t num_locks = ds_stress_test_num_locks[l];
			for(size_t p = 0; p < sizeof(ds_stress_test_lock_pattern)/sizeof(ds_stress_test_lock_pattern[0]); p++) {
				int pattern = ds_stress_test_lock_pattern[p];

				Ds* locks = new Ds[num_locks];

				typename Pheet::Environment::PerformanceCounters pc;
				{typename Pheet::Environment env(cpus, pc);

					Ds ds;

					switch(type) {
					case 0: // Throughput
						run_throughput_test(locks, num_locks, pattern);
						break;
					}
				}
				std::cout << "test\ttype\tds\tscheduler\tnum_locks\tseed\tcpus\t";
				switch(type) {
				case 0: // Throughput
					std::cout << "throughput\tthroughput min\tthroughput max\t";
					break;
				}
				Pheet::Environment::PerformanceCounters::print_headers();
				std::cout << std::endl;
				switch(type) {
				case 0: // Throughput
					std::cout << "Throughput\t";
					switch(pattern) {
					case 0:
						std::cout << "single access\t";
						break;
					case 1:
						std::cout << "random subarray (uniform)\t";
						break;
					case 2:
						std::cout << "random subarray (high congestion)\t";
						break;
					default:
						std::cout << "unsupported\t";
						break;
					}
					break;
				default:
					std::cout << "unsupported\t\t";
					break;
				}
				Ds::print_name();
				std::cout << "\t";
				Pheet::Environment::print_name();
				std::cout << "\t" << num_locks << "\t" << seed << "\t" << cpus << "\t";
				switch(type) {
				case 0: // Throughput
					std::cout << throughput << "\t"
						<< throughput_min << "\t"
						<< throughput_max << "\t";
					break;
				}
				pc.print_values();
				std::cout << std::endl;
			}
		}
	}

	void run_throughput_test(Ds* locks, size_t num_locks, int pattern) {
		SumReducer<Pheet, size_t> throughput;
		MinReducer<Pheet, size_t> throughput_min;
		MaxReducer<Pheet, size_t> throughput_max;

		std::atomic<int> phase(0);
		std::atomic<procs_t> ready(cpus);
		std::chrono::high_resolution_clock::time_point start_time;

		Pheet::template
			finish<Throughput>(locks, num_locks, pattern, cpus,
					throughput, throughput_min,	throughput_max,
					phase, ready, start_time, seed);

		this->throughput = throughput.get_sum();
		this->throughput_min = throughput_min.get_min();
		this->throughput_max = throughput_max.get_max();
	}

private:
	procs_t cpus;
	int type;
	unsigned int seed;

	size_t throughput;
	size_t throughput_min;
	size_t throughput_max;
};

} /* namespace pheet */
#endif /* LOCKTEST_H_ */
