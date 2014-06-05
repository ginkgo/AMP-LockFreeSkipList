/*
 * SsspTest.h
 *
 *  Created on: Jul 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SSSPTEST_H_
#define SSSPTEST_H_

#include "sssp_graph_helpers.h"

#include <pheet/pheet.h>
#include "../Test.h"

#include <stdlib.h>
#include <vector>
#include <iostream>
#include <random>
#include <limits>

#include "SsspInitTask.h"

#include <pheet/sched/Synchroneous/SynchroneousScheduler.h>

namespace pheet {

template <class Pheet, template <class P> class Algorithm>
class SsspTest : Test {
public:
	SsspTest(procs_t cpus, int type, size_t k, size_t size, float p, size_t max_w, unsigned int seed);
	~SsspTest();

	void run_test();

private:
	SsspGraphVertex* generate_data();
	void delete_data(SsspGraphVertex* data);
	bool check_solution(SsspGraphVertex* data);
	static void check_vertex(SsspGraphVertex* data, size_t i, bool& correct);

	procs_t cpus;
	int type;
	size_t k;
	size_t size;
	float p;
	size_t max_w;
	unsigned int seed;

	static char const* const types[];
};

template <class Pheet, template <class P> class Algorithm>
char const* const SsspTest<Pheet, Algorithm>::types[] = {"random"};

template <class Pheet, template <class P> class Algorithm>
SsspTest<Pheet, Algorithm>::SsspTest(procs_t cpus, int type, size_t k, size_t size, float p, size_t max_w, unsigned int seed)
: cpus(cpus), type(type), k(k), size(size), p(p), max_w(max_w), seed(seed) {

}

template <class Pheet, template <class P> class Algorithm>
SsspTest<Pheet, Algorithm>::~SsspTest() {

}

template <class Pheet, template <class P> class Algorithm>
void SsspTest<Pheet, Algorithm>::run_test() {
	SsspGraphVertex* data = generate_data();

	typename Pheet::Environment::PerformanceCounters pc;
	typename Algorithm<Pheet>::PerformanceCounters ppc;

	Time start, end;
	{typename Pheet::Environment env(cpus, pc);
		Algorithm<Pheet>::set_k(k);
		check_time(start);
		Pheet::template
			finish<Algorithm<Pheet> >(data, size, ppc);
		check_time(end);
	}

	bool correct = check_solution(data);
	double seconds = calculate_seconds(start, end);
	std::cout << "test\talgorithm\tscheduler\ttype\tsize\tp\tmax_w\tk\tseed\tcpus\ttotal_time\tcorrect\t";
//	Algorithm<Pheet>::print_headers();
	Algorithm<Pheet>::PerformanceCounters::print_headers();
	Pheet::Environment::PerformanceCounters::print_headers();
	std::cout << std::endl;
	std::cout << "sssp\t";
	Algorithm<Pheet>::print_name();
	std::cout << "\t";
	Pheet::Environment::print_name();
	std::cout << "\t" << types[type] << "\t" << size << "\t" << p << "\t" << max_w << "\t" << k << "\t" << seed << "\t" << cpus << "\t" << seconds << "\t" << correct << "\t";
//	Algorithm<Pheet>::print_configuration();
	ppc.print_values();
	pc.print_values();
	std::cout << std::endl;

	delete_data(data);
}

template <class Pheet, template <class P> class Algorithm>
SsspGraphVertex* SsspTest<Pheet, Algorithm>::generate_data() {
	SsspGraphVertex* data = new SsspGraphVertex[size];

	std::mt19937 rng;
	rng.seed(seed);
    std::uniform_real_distribution<float> rnd_f(0.0, 1.0);
    std::uniform_int_distribution<size_t> rnd_st(1, max_w);

	std::vector<SsspGraphEdge>* edges = new std::vector<SsspGraphEdge>[size];
	for(size_t i = 0; i < size; ++i) {
		for(size_t j = i + 1; j < size; ++j) {
			if(rnd_f(rng) < p) {
				SsspGraphEdge e;
				e.target = j;
				e.weight = rnd_st(rng);
				edges[i].push_back(e);
				e.target = i;
				edges[j].push_back(e);
			}
		}
		data[i].num_edges = edges[i].size();
		if(edges[i].size() > 0) {
			data[i].edges = new SsspGraphEdge[edges[i].size()];
			for(size_t j = 0; j < edges[i].size(); ++j) {
				data[i].edges[j] = edges[i][j];
			}
		}
		else {
			data[i].edges = NULL;
		}
		data[i].distance = std::numeric_limits<size_t>::max();
	}
/*	{typedef typename Pheet::template WithScheduler<SynchroneousScheduler> SPheet;
		typedef typename SPheet::Environment SEnv;
		// For some reason the default constructor is not called with GCC, that's why we specify the number of CPUs
		SEnv env(1);

		SPheet::template
			finish<SsspInitTask<SPheet> >(data, edges, 0, size - 1);
	}*/
	data[0].distance = 0;
	delete[] edges;

	return data;
}

template <class Pheet, template <class P> class Algorithm>
void SsspTest<Pheet, Algorithm>::delete_data(SsspGraphVertex* data) {
	for(size_t i = 0; i < size; ++i) {
		if(data[i].edges != NULL) {
			delete[] data[i].edges;
		}
	}
	delete[] data;
}

template <class Pheet, template <class P> class Algorithm>
bool SsspTest<Pheet, Algorithm>::check_solution(SsspGraphVertex* data) {
	bool correct = true;
	{pheet::Pheet::Environment p;
		for(size_t i = 0; i < size; ++i) {
			pheet::Pheet::spawn(SsspTest<Pheet, Algorithm>::check_vertex, data, i, correct);
		}
	}

	return correct;
}

template <class Pheet, template <class P> class Algorithm>
void SsspTest<Pheet, Algorithm>::check_vertex(SsspGraphVertex* data, size_t i, bool& correct) {
	for(size_t j = 0; j < data[i].num_edges; ++j) {
		if(data[data[i].edges[j].target].distance > data[i].distance + data[i].edges[j].weight) {
			correct = false;
			return;
		}
	}
}

} /* namespace pheet */
#endif /* SSSPTEST_H_ */
