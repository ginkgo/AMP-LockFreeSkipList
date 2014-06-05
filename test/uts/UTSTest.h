/*
* UTSTests.h
*
*  Created on: 5.12.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef UTSTEST_H_
#define UTSTEST_H_

#include "../settings.h"
#include "../../pheet/misc/types.h"
#include "../Test.h"
#include "RecursiveSearch/uts.h"
#undef max
#undef min

#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

namespace pheet {

template <class Impl>
class UTSTest : Test {
public:
	UTSTest(procs_t cpus, string command);
	~UTSTest();

	void run_test();

private:
	procs_t cpus;
	string command;
};

template <class Impl>
UTSTest<Impl>::UTSTest(procs_t cpus, string command)
: cpus(cpus),command(command){

}

template <class Impl>
UTSTest<Impl>::~UTSTest() {

}

template <class Impl>
void UTSTest<Impl>::run_test() {

	Impl iar(cpus);

	std::istringstream ss(command);
	std::string arg;
	std::vector<std::string> v1;
	std::vector<char*> v2;
	while (ss >> arg)
	{
	   v1.push_back(arg);
	   v2.push_back(const_cast<char*>(v1.back().c_str()));
	}
	v2.push_back(0);

	uts_parseParams(v2.size()-1,&v2[0]);

	Time start, end;
	check_time(start);
	iar.run();
	check_time(end);

	double seconds = calculate_seconds(start, end);
	std::cout << "test\tworkload\timplementation\tscheduler\tcpus\ttotal_time\t";
	iar.print_headers();
	std::cout << std::endl;
	std::cout << "uts\t";
	std::cout << v1[0] << "\t";
	Impl::print_name();
	std::cout << "\t";
	Impl::print_scheduler_name();
	std::cout << "\t" << cpus << "\t" << seconds << "\t";
	iar.print_results();
	std::cout << std::endl;
}
}

#endif /* UTSTESTS_H_ */
