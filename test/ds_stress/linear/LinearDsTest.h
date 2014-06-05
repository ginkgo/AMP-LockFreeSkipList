/*
 * LinearDsTest.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LINEARDSTEST_H_
#define LINEARDSTEST_H_

#include <pheet/pheet.h>
#include "../../Test.h"
#include <random>
#include <chrono>

#include "LinearDsInitTask.h"
#include "LinearDsThroughputTestTask.h"
#include "LinearDsCorrectnessTestTask.h"
#include <pheet/primitives/Reducer/Sum/SumReducer.h>
#include <pheet/primitives/Reducer/Min/MinReducer.h>
#include <pheet/primitives/Reducer/Max/MaxReducer.h>


namespace pheet {

template <class Pheet, template <class, typename> class DsT>
class LinearDsTest {
public:
	typedef DsT<Pheet, unsigned int> Ds;

	typedef LinearDsInitTask<Pheet, Ds> InitTask;
	typedef LinearDsThroughputTestTask<Pheet, Ds> Throughput;
	typedef LinearDsCorrectnessTestTask<Pheet, Ds> CorrectnessTest;

	LinearDsTest(procs_t cpus, int type, unsigned int seed)
	: cpus(cpus), type(type), seed(seed) {

	}

	~LinearDsTest() {}

	void run_test() {
		for(size_t d = 0; d < sizeof(ds_stress_test_linear_dist)/sizeof(ds_stress_test_linear_dist[0]); d++) {
			int dist = ds_stress_test_linear_dist[d];
			if(type == 1) {
				run_correctness_test(dist);
			} else {
				for(size_t p = 0; p < sizeof(ds_stress_test_prefill)/sizeof(ds_stress_test_prefill[0]); p++) {
					if(dist == 1 && cpus < 2) {
						// 50:50 dedicated only makes sense with 2 or more cpus
						continue;
					}

					size_t prefill = ds_stress_test_prefill[p];

					typename Pheet::Environment::PerformanceCounters pc;
					{typename Pheet::Environment env(cpus, pc);

						Ds ds;

						Pheet::template
							finish<InitTask>(ds, prefill, seed);

						switch(type) {
						case 0: // Throughput
							run_throughput_test(ds, ds_stress_test_linear_dist[d]);
							break;
						}
					}
					std::cout << "test\ttype\tds\tscheduler\tprefill\tseed\tcpus\t";
					switch(type) {
					case 0: // Throughput
						std::cout << "push throughput\tpop throughput\tpush throughput min\tpop throughput min\tpush throughput max\tpop throughput max\t";
						break;
					}
					Pheet::Environment::PerformanceCounters::print_headers();
					std::cout << std::endl;
					switch(type) {
					case 0: // Throughput
						std::cout << "Throughput\t";
						switch(dist) {
						case 0:
							std::cout << "50:50\t";
							break;
						case 1:
							std::cout << "50:50d\t";
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
						std::cout << push_throughput << "\t" << pop_throughput << "\t"
							<< push_throughput_min << "\t" << pop_throughput_min << "\t"
							<< push_throughput_max << "\t" << pop_throughput_max << "\t";
						break;
					}
					pc.print_values();
					std::cout << std::endl;
				}
			}
		}
	}

	void run_throughput_test(Ds& ds, int dist) {
		SumReducer<Pheet, size_t> push_throughput;
		MinReducer<Pheet, size_t> push_throughput_min;
		MaxReducer<Pheet, size_t> push_throughput_max;
		SumReducer<Pheet, size_t> pop_throughput;
		MinReducer<Pheet, size_t> pop_throughput_min;
		MaxReducer<Pheet, size_t> pop_throughput_max;

		std::atomic<int> phase(0);
		std::atomic<procs_t> ready(cpus);
		std::chrono::high_resolution_clock::time_point start_time;

		Pheet::template
			finish<Throughput>(ds, dist, cpus, push_throughput, pop_throughput,
					push_throughput_min, pop_throughput_min,
					push_throughput_max, pop_throughput_max,
					phase, ready, start_time, seed);

		this->push_throughput = push_throughput.get_sum();
		this->push_throughput_min = push_throughput_min.get_min();
		this->push_throughput_max = push_throughput_max.get_max();
		this->pop_throughput = pop_throughput.get_sum();
		this->pop_throughput_min = pop_throughput_min.get_min();
		this->pop_throughput_max = pop_throughput_max.get_max();
	}

	void run_correctness_test(int dist) {
		for(size_t ni = 0; ni < sizeof(ds_stress_test_linear_correctness_n)/sizeof(ds_stress_test_linear_correctness_n[0]); ni++) {
			size_t n = ds_stress_test_linear_correctness_n[ni];
			for(size_t p = 0; p < sizeof(ds_stress_test_linear_correctness_prefill_p)/sizeof(ds_stress_test_linear_correctness_prefill_p[0]); p++) {
				double pp = ds_stress_test_linear_correctness_prefill_p[p];


				typename Pheet::Environment::PerformanceCounters pc;
				{typename Pheet::Environment env(cpus, pc);

					Ds ds[2];

					LinearDsCorrectnessTestEvent* history = new LinearDsCorrectnessTestEvent[n * cpus];

					std::atomic<int> phase(0);
					std::atomic<procs_t> ready(cpus);

					Pheet::template
						finish<CorrectnessTest>(ds, history, n, pp, dist, cpus, cpus, phase, ready, seed);

					// TODO: Verify correctness

					delete[] history;
				}
				std::cout << "test\ttype\tds\tscheduler\tn\tprefill_p\tseed\tcpus\t";
				Pheet::Environment::PerformanceCounters::print_headers();
				std::cout << std::endl;
				std::cout << "Correctness" << "\t";
				Ds::print_name();
				std::cout << "\t";
				Pheet::Environment::print_name();
				std::cout << "\t" << n << "\t" << pp << "\t" << seed << "\t" << cpus << "\t";
				pc.print_values();
				std::cout << std::endl;
			}
		}

	}

private:
	procs_t cpus;
	int type;
	unsigned int seed;

	size_t push_throughput;
	size_t push_throughput_min;
	size_t push_throughput_max;
	size_t pop_throughput;
	size_t pop_throughput_min;
	size_t pop_throughput_max;
};

} /* namespace pheet */
#endif /* LINEARDSTEST_H_ */
