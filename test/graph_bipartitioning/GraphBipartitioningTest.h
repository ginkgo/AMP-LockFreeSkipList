/*
 * GraphBipartitioningTest.h
 *
 *  Created on: 07.09.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef GRAPHBIPARTITIONINGTEST_H_
#define GRAPHBIPARTITIONINGTEST_H_

#include "graph_helpers.h"

#include "pheet/pheet.h"
#include "../Test.h"

#include <stdlib.h>
#include <vector>
#include <iostream>
#include <random>

/*
 *
 */
namespace pheet {

template <class Pheet, template <class P> class Partitioner>
class GraphBipartitioningTest : Test {
public:
	GraphBipartitioningTest(procs_t cpus, int type, size_t size, float p, size_t max_w, unsigned int seed);
	~GraphBipartitioningTest();

	void run_test();

private:
	GraphVertex* generate_data();
	void delete_data(GraphVertex* data);
	size_t check_solution(GraphVertex* data, typename Partitioner<Pheet>::Solution const& solution);

	procs_t cpus;
	int type;
	size_t const size;
	float p;
	size_t max_w;
	unsigned int seed;

	static char const* const types[];
};

template <class Pheet, template <class P> class Partitioner>
char const* const GraphBipartitioningTest<Pheet, Partitioner>::types[] = {"random"};

template <class Pheet, template <class P> class Partitioner>
GraphBipartitioningTest<Pheet, Partitioner>::GraphBipartitioningTest(procs_t cpus, int type, size_t size, float p, size_t max_w, unsigned int seed)
: cpus(cpus), type(type), size(size), p(p), max_w(max_w), seed(seed) {

}

template <class Pheet, template <class P> class Partitioner>
GraphBipartitioningTest<Pheet, Partitioner>::~GraphBipartitioningTest() {

}

template <class Pheet, template <class P> class Partitioner>
void GraphBipartitioningTest<Pheet, Partitioner>::run_test() {
	GraphVertex* data = generate_data();

	typename Pheet::Environment::PerformanceCounters pc;
	typename Partitioner<Pheet>::PerformanceCounters ppc;
	typename Partitioner<Pheet>::Solution solution;

	Time start, end;
	{typename Pheet::Environment env(cpus, pc);
		check_time(start);
		ppc.subproblem_pc.last_update_time.start_timer();

		Pheet::template
			finish<Partitioner<Pheet> >(data, size, solution, ppc);
		check_time(end);
	}

	size_t weight = check_solution(data, solution);
	double seconds = calculate_seconds(start, end);
	std::cout << "test\tpartitioner\tscheduler\ttype\tsize\tp\tmax_w\tseed\tcpus\ttotal_time\tweight\t";
	Partitioner<Pheet>::print_headers();
	Partitioner<Pheet>::PerformanceCounters::print_headers();
	Pheet::Environment::PerformanceCounters::print_headers();
	std::cout << std::endl;
	std::cout << "graph_bipartitioning\t" << Partitioner<Pheet>::name << "\t";
	Pheet::Environment::print_name();
	std::cout << "\t" << types[type] << "\t" << size << "\t" << p << "\t" << max_w << "\t" << seed << "\t" << cpus << "\t" << seconds << "\t" << weight << "\t";
	Partitioner<Pheet>::print_configuration();
	ppc.print_values();
	pc.print_values();
	std::cout << std::endl;

	delete_data(data);
}

// JLT: would like to have more generators - template?
template <class Pheet, template <class P> class Partitioner>
GraphVertex* GraphBipartitioningTest<Pheet, Partitioner>::generate_data() {
	GraphVertex* data = new GraphVertex[size];

	std::mt19937 rng;
	rng.seed(seed);
    std::uniform_real_distribution<float> rnd_f(0.0, 1.0);
    std::uniform_int_distribution<size_t> rnd_st(1, max_w);

	std::vector<GraphEdge>* edges = new std::vector<GraphEdge>[size];
	for(size_t i = 0; i < size; ++i) {
		for(size_t j = i + 1; j < size; ++j) {
			if(rnd_f(rng) < p) {
				GraphEdge e;
				e.target = j;
				e.weight = rnd_st(rng);
				edges[i].push_back(e);
				e.target = i;
				edges[j].push_back(e);
			}
		}
		data[i].num_edges = edges[i].size();
		if(edges[i].size() > 0) {
			data[i].edges = new GraphEdge[edges[i].size()];
			for(size_t j = 0; j < edges[i].size(); ++j) {
				data[i].edges[j] = edges[i][j];
			}
			//std::sort(&data[i].edges[0],&data[i].edges[data[i].num_edges],edgeweight_compare());
			std::sort(data[i].edges,data[i].edges+data[i].num_edges,edgeweight_compare());
			/*
			for (size_t j=0;  j < data[i].num_edges; j++) {
			  std::cout << data[i].edges[j].weight << ' ';
			}
			std::cout << '\n' << '\n';
			*/
		}
		else {
			data[i].edges = NULL;
		}
	}
	delete[] edges;

	// create reverse edges, O(n+m), space O(n^2)
	// should be factored out
	size_t* eix = new size_t[size * size];
	for (size_t i = 0; i < size; i++) {
	  for (size_t j = 0; j < data[i].num_edges; j++) {
	    eix[i*size + data[i].edges[j].target] = j;
	  }
	}
	for (size_t i = 0; i < size; i++) {
	  for (size_t j = 0; j < data[i].num_edges; j++) {
	    data[i].edges[j].reverse = eix[data[i].edges[j].target*size + i];

	    //std::cout << '(' << i << ',' << data[i].edges[j].target << "):" << data[i].edges[j].reverse << ' ';
	
	  }
	  //std::cout << '\n' << '\n';
	}
	delete[] eix;

	return data;
}

template <class Pheet, template <class P> class Partitioner>
void GraphBipartitioningTest<Pheet, Partitioner>::delete_data(GraphVertex* data) {
	for(size_t i = 0; i < size; ++i) {
		if(data[i].edges != NULL) {
			delete[] data[i].edges;
		}
	}
	delete[] data;
}

template <class Pheet, template <class P> class Partitioner>
size_t GraphBipartitioningTest<Pheet, Partitioner>::check_solution(GraphVertex* data, typename Partitioner<Pheet>::Solution const& solution) {
	size_t k = size >> 1;

	if(solution.sets[0].count() != k) {
		std::cout << "invalid solution" << std::endl;
	}
	if(solution.sets[1].count() != size - k) {
		std::cout << "invalid solution" << std::endl;
	}

	size_t weight = 0;
	size_t current_bit = solution.sets[0]._Find_first();
	while(current_bit != solution.sets[0].size()) {
		for(size_t i = 0; i < data[current_bit].num_edges; ++i) {
			if(solution.sets[1].test(data[current_bit].edges[i].target)) {
				pheet_assert(!solution.sets[0].test(data[current_bit].edges[i].target));
				weight += data[current_bit].edges[i].weight;
			}
			else {
				pheet_assert(solution.sets[0].test(data[current_bit].edges[i].target));
			}
		}
		current_bit = solution.sets[0]._Find_next(current_bit);
	}

	if(weight != solution.weight) {
		std::cout << "weight doesn't match" << std::endl;
	}

	return solution.weight;
}

}

#endif /* GRAPHBIPARTITIONINGTEST_H_ */
