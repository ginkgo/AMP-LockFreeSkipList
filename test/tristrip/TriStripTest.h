/*
* TriStripTests.h
*
*  Created on: 5.12.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef TriStripTEST_H_
#define TriStripTEST_H_

#include "../../pheet/settings.h"
#include "../../pheet/misc/types.h"
#include "../Test.h"
#include "GraphDual.h"

#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

namespace pheet {

template <class Pheet, template <class> class ImplT>
class TriStripTest : Test {
public:
	typedef ImplT<Pheet> Impl;
    TriStripTest(procs_t cpus, size_t nodecount, unsigned int seed, bool withstrat);
	~TriStripTest();

	void run_test();

private:
	procs_t cpus;
	size_t nodecount;
	unsigned int seed;
	bool withstrat;
};

template <class Pheet, template <class> class Impl>
  TriStripTest<Pheet, Impl>::TriStripTest(procs_t cpus, size_t nodecount, unsigned int seed, bool withstrat)
  : cpus(cpus),nodecount(nodecount), seed(seed), withstrat(withstrat){

}

template <class Pheet, template <class> class Impl>
TriStripTest<Pheet, Impl>::~TriStripTest() {

}

template <class Pheet, template <class> class Impl>
void TriStripTest<Pheet, Impl>::run_test() {

	GraphDual graph(nodecount,seed,0.4);

//	printf("Start\n");

	Impl iar(cpus, graph, withstrat);
	typename Pheet::Environment::PerformanceCounters pc;


	Time start, end;
	{typename Pheet::Environment env(cpus, pc);
		check_time(start);
		iar.run();
		check_time(end);
	}

	double seconds = calculate_seconds(start, end);
	std::cout << "test\timplementation\tscheduler\tcpus\ttotal_time\ttristrips\tnodesintristrips\t";
	iar.print_headers();
	Pheet::Environment::PerformanceCounters::print_headers();
	std::cout << std::endl;
	std::cout << "tristrip\t";
	Impl::print_name();
	std::cout << "\t";
	Impl::print_scheduler_name();
	std::cout << "\t" << cpus << "\t" << seconds << "\t";
	std::cout << /*nodecount << "\t" <<*/ iar.getTriStripCount() << "\t" << iar.getNodeTriStripCount() << "\t";
	iar.print_results();
	pc.print_values();
	std::cout << std::endl;
}
}

#endif /* TriStripTESTS_H_ */
