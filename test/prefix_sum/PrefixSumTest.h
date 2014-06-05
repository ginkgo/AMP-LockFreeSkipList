/*
 * PrefixSumTest.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef PREFIXSUMTEST_H_
#define PREFIXSUMTEST_H_

#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <pheet/pheet.h>
#include <pheet/misc/align.h>
#include "../Test.h"

#include "PrefixSumInitTask.h"

//using namespace std;

namespace pheet {



template <class Pheet, template <class P> class Algorithm>
class PrefixSumTest : Test {
public:
	PrefixSumTest(procs_t cpus, size_t num_problems, int type, size_t size, unsigned int seed);
	virtual ~PrefixSumTest();

	void run_test();

private:
	typedef aligned_data<unsigned int, 64> TestData;

	void generate_data(TestData*);
	bool is_correct(unsigned int* data);

	procs_t cpus;
	size_t num_problems;
	int type;
	size_t size;
	unsigned int seed;

	static char const* const types[];
};

template <class Pheet, template <class P> class Algorithm>
char const* const PrefixSumTest<Pheet, Algorithm>::types[] = {"random", "gauss", "equal", "bucket", "staggered"};

template <class Pheet, template <class P> class Algorithm>
PrefixSumTest<Pheet, Algorithm>::PrefixSumTest(procs_t cpus, size_t num_problems, int type, size_t size, unsigned int seed)
: cpus(cpus), num_problems(num_problems), type(type), size(size), seed(seed) {

}

template <class Pheet, template <class P> class Algorithm>
PrefixSumTest<Pheet, Algorithm>::~PrefixSumTest() {

}

template <class Pheet, template <class P> class Algorithm>
void PrefixSumTest<Pheet, Algorithm>::run_test() {
	TestData* data = new TestData[num_problems];
	generate_data(data);

	typename Pheet::Environment::PerformanceCounters pc;
	typename Algorithm<Pheet>::PerformanceCounters apc;

	Time start, end;

	{typename Pheet::Environment env(cpus, pc);
		check_time(start);
		{typename Pheet::Finish f;
			for(size_t i = 0; i < num_problems; ++i) {
				Pheet::template
					spawn<Algorithm<Pheet> >(data[i].ptr(), size, apc);
			}
		}

		check_time(end);
	}

	bool correctness = true;
	for(size_t i = 0; i < num_problems; ++i) {
		correctness &= is_correct(data[i].ptr());
	}
	double seconds = calculate_seconds(start, end);
	std::cout << "test\talgorithm\tscheduler\tnum_problems\ttype\tsize\tseed\tcpus\ttotal_time\tcorrect\t";
	Pheet::Environment::PerformanceCounters::print_headers();
	Algorithm<Pheet>::PerformanceCounters::print_headers();
	std::cout << std::endl;
	std::cout << "prefix_sum\t" << Algorithm<Pheet>::name << "\t";
	Pheet::Environment::print_name();
	std::cout << "\t" << num_problems << "\t" << types[type] << "\t" << size << "\t" << seed << "\t" << cpus << "\t" << seconds << "\t" << correctness << "\t";
	pc.print_values();
	apc.print_values();
	std::cout << std::endl;
	
	delete[] data;
}


template <class Pheet, template <class P> class Algorithm>
void PrefixSumTest<Pheet, Algorithm>::generate_data(TestData* data) {
	typename Pheet::Environment env(cpus);
	for(size_t i = 0; i < num_problems; ++i) {
		TestData testData(size);
		unsigned int* d = testData.ptr();

		Pheet::template finish<PrefixSumInitTask<Pheet> >(d, size, type);
		data[i] = std::move(testData);
	}
}

template <class Pheet, template <class P> class Algorithm>
bool PrefixSumTest<Pheet, Algorithm>::is_correct(unsigned int* data) {
	for(size_t i = 0; i < size; i++) {
		if(data[i] != i + 1) {
			std::cout << ">>" << i << ": " << data[i] << std::endl;
			return false;
		}
	}
	return true;
}

} /* namespace pheet */
#endif /* PREFIXSUMTEST_H_ */
