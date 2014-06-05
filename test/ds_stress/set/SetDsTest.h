/*
 * SetDsTest.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SETDSTEST_H_
#define SETDSTEST_H_

#include <pheet/pheet.h>
#include "../../Test.h"
#include <random>
#include <chrono>

#include "SetDsInitTask.h"
#include "SetDsThroughputTestTask.h"
#include <pheet/primitives/Reducer/Sum/SumReducer.h>
#include <pheet/primitives/Reducer/Min/MinReducer.h>
#include <pheet/primitives/Reducer/Max/MaxReducer.h>


namespace pheet {

template <class Pheet, template <class, typename> class DsT>
class SetDsTest {
public:
	typedef DsT<Pheet, unsigned int> Ds;

	typedef SetDsInitTask<Pheet, Ds> InitTask;
	typedef SetDsThroughputTestTask<Pheet, Ds> Throughput;

	SetDsTest(procs_t cpus, int type, unsigned int seed)
	: cpus(cpus), type(type), seed(seed) {

	}

	~SetDsTest() {}

	void run_test() {
		for(size_t p = 0; p < sizeof(ds_stress_test_prefill)/sizeof(ds_stress_test_prefill[0]); p++) {
			for(size_t d = 0; d < sizeof(ds_stress_test_set_dist)/sizeof(ds_stress_test_set_dist[0]); d++) {
				int dist = ds_stress_test_set_dist[d];
				for(size_t m = 0; m < sizeof(ds_stress_test_set_max_v)/sizeof(ds_stress_test_set_max_v[0]); m++) {
					unsigned int max_v = ds_stress_test_set_max_v[m];

					if((dist == 1 || dist == 3) && cpus < 2) {
						// 50:50 dedicated only makes sense with 2 or more cpus
						continue;
					}

					size_t prefill = ds_stress_test_prefill[p];
					if(prefill > (max_v >> 1)) {
						continue;
					}

					typename Pheet::Environment::PerformanceCounters pc;
					{typename Pheet::Environment env(cpus, pc);

						Ds ds;

						Pheet::template
							finish<InitTask>(ds, prefill, max_v, seed);

						switch(type) {
						case 0: // Throughput
							run_throughput_test(ds, dist, max_v);
							break;
						}
					}
					std::cout << "test\ttype\tds\tscheduler\tprefill\tseed\tcpus\t";
					switch(type) {
					case 0: // Throughput
						std::cout << "add throughput\tremove throughput\tcontains throughput\tadd throughput min\tremove throughput min\tcontains throughput min\tadd throughput max\tremove throughput max\tcontains throughput max\t";
						break;
					}
					Pheet::Environment::PerformanceCounters::print_headers();
					std::cout << std::endl;
					switch(type) {
					case 0: // Throughput
						std::cout << "Throughput\t";
						switch(dist) {
						case 0:
							std::cout << "1:1:2\t";
							break;
						case 1:
							std::cout << "1:1:2d\t";
							break;
						case 2:
							std::cout << "1:1:98\t";
							break;
						case 3:
							std::cout << "1:1:P-2d\t";
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
					std::cout << "\t" << prefill << "\t" << seed << "\t" << cpus << "\t";
					switch(type) {
					case 0: // Throughput
						std::cout << add_throughput << "\t" << remove_throughput << "\t" << contains_throughput << "\t"
							<< add_throughput_min << "\t" << remove_throughput_min << "\t" << contains_throughput_min << "\t"
							<< add_throughput_max << "\t" << remove_throughput_max << "\t" << contains_throughput_max << "\t";
						break;
					}
					pc.print_values();
					std::cout << std::endl;
				}
			}
		}
	}

	void run_throughput_test(Ds& ds, int dist, unsigned int max_v) {
		SumReducer<Pheet, size_t> add_throughput;
		MinReducer<Pheet, size_t> add_throughput_min;
		MaxReducer<Pheet, size_t> add_throughput_max;
		SumReducer<Pheet, size_t> remove_throughput;
		MinReducer<Pheet, size_t> remove_throughput_min;
		MaxReducer<Pheet, size_t> remove_throughput_max;
		SumReducer<Pheet, size_t> contains_throughput;
		MinReducer<Pheet, size_t> contains_throughput_min;
		MaxReducer<Pheet, size_t> contains_throughput_max;

		std::atomic<int> phase(0);
		std::atomic<procs_t> ready(cpus);
		std::chrono::high_resolution_clock::time_point start_time;

		Pheet::template
			finish<Throughput>(ds, dist, max_v, cpus, add_throughput, remove_throughput, contains_throughput,
					add_throughput_min, remove_throughput_min, contains_throughput_min,
					add_throughput_max, remove_throughput_max, contains_throughput_max,
					phase, ready, start_time, seed);

		this->add_throughput = add_throughput.get_sum();
		this->add_throughput_min = add_throughput_min.get_min();
		this->add_throughput_max = add_throughput_max.get_max();
		this->remove_throughput = remove_throughput.get_sum();
		this->remove_throughput_min = remove_throughput_min.get_min();
		this->remove_throughput_max = remove_throughput_max.get_max();
		this->contains_throughput = contains_throughput.get_sum();
		this->contains_throughput_min = contains_throughput_min.get_min();
		this->contains_throughput_max = contains_throughput_max.get_max();
	}

private:
	procs_t cpus;
	int type;
	unsigned int seed;

	size_t add_throughput;
	size_t add_throughput_min;
	size_t add_throughput_max;
	size_t remove_throughput;
	size_t remove_throughput_min;
	size_t remove_throughput_max;
	size_t contains_throughput;
	size_t contains_throughput_min;
	size_t contains_throughput_max;
};

} /* namespace pheet */
#endif /* SETDSTEST_H_ */
