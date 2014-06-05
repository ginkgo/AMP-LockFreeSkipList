/*
 * InARowTest.h
 *
 *  Created on: Nov 23, 2011
 *      Author: Martin Wimmer
 */

#ifndef INAROWTEST_H_
#define INAROWTEST_H_

#include "../settings.h"
#include "../../pheet/misc/types.h"
#include "../Test.h"

#include <iostream>

namespace pheet {

template <class Impl>
class InARowTest : Test {
public:
	InARowTest(procs_t cpus, unsigned int width, unsigned int height, unsigned int rowlength, unsigned int lookahead, unsigned int* scenario);
	~InARowTest();

	void run_test();

private:
	procs_t cpus;
	unsigned int width;
	unsigned int height;
	unsigned int rowlength;
	unsigned int lookahead;
	unsigned int* scenario;
};

template <class Impl>
InARowTest<Impl>::InARowTest(procs_t cpus, unsigned int width, unsigned int height, unsigned int rowlength, unsigned int lookahead, unsigned int* scenario)
: cpus(cpus), width(width), height(height), rowlength(rowlength), lookahead(lookahead), scenario(scenario) {

}

template <class Impl>
InARowTest<Impl>::~InARowTest() {

}

template <class Impl>
void InARowTest<Impl>::run_test() {
	Impl iar(cpus, width, height, rowlength, lookahead, scenario);

	Time start, end;
	check_time(start);
	iar.run();
	check_time(end);

	double seconds = calculate_seconds(start, end);
	std::cout << "test\timplementation\tscheduler\twidth\theight\trowlength\tlookahead\tcpus\ttotal_time\t";
	iar.print_headers();
	std::cout << std::endl;
	std::cout << "inarow\t";
	Impl::print_name();
	std::cout << "\t";
	Impl::print_scheduler_name();
	std::cout << "\t" << width << "\t" << height << "\t" << rowlength << "\t" << lookahead << "\t" << cpus << "\t" << seconds << "\t";
	iar.print_results();
	std::cout << std::endl;
}

}

#endif /* INAROWTEST_H_ */
