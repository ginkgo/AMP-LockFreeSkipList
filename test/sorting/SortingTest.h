/*
 * SortingTestBase.h
 *
 *  Created on: 07.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SORTINGTEST_H_
#define SORTINGTEST_H_

#include <stdlib.h>
#include <iostream>
#include <pheet/pheet.h>
#include "../Test.h"
#include <exception>

#include <random>

namespace pheet {

template <class Pheet, template <class P> class Sorter>
class SortingTest : Test {
public:
	SortingTest(procs_t cpus, int type, size_t size, unsigned int seed);
	virtual ~SortingTest();

	void run_test();

private:
	unsigned int* generate_data();
	size_t get_number_of_runs(unsigned int* data);

	procs_t cpus;
	int type;
	size_t size;
	unsigned int seed;

	static char const* const types[];
};

template <class Pheet, template <class P> class Sorter>
char const* const SortingTest<Pheet, Sorter>::types[] = {"random", "gauss", "equal", "bucket", "staggered", "ascending", "descending"};

template <class Pheet, template <class P> class Sorter>
SortingTest<Pheet, Sorter>::SortingTest(procs_t cpus, int type, size_t size, unsigned int seed)
: cpus(cpus), type(type), size(size), seed(seed) {

}

template <class Pheet, template <class P> class Sorter>
SortingTest<Pheet, Sorter>::~SortingTest() {

}

template <class Pheet, template <class P> class Sorter>
void SortingTest<Pheet, Sorter>::run_test() {
	unsigned int* data = generate_data();

	typename Pheet::Environment::PerformanceCounters pc;

	Time start, end;

	{typename Pheet::Environment env(cpus, pc);
		check_time(start);
		Pheet::template
			finish<Sorter<Pheet> >(data, size);
		check_time(end);
	}

	size_t runs = get_number_of_runs(data);
	double seconds = calculate_seconds(start, end);
	std::cout << "test\tsorter\tscheduler\ttype\tsize\tseed\tcpus\ttotal_time\truns\t";
	Pheet::Environment::PerformanceCounters::print_headers();
	std::cout << std::endl;
	std::cout << "sorting\t" << Sorter<Pheet>::name << "\t";
	Pheet::Environment::print_name();
	std::cout << "\t" << types[type] << "\t" << size << "\t" << seed << "\t" << cpus << "\t" << seconds << "\t" << runs << "\t";
	pc.print_values();
	std::cout << std::endl;
	delete[] data;
}


template <class Pheet, template <class P> class Sorter>
unsigned int* SortingTest<Pheet, Sorter>::generate_data() {
	unsigned int* data = new unsigned int[size];

	std::mt19937 rng;
	rng.seed(seed);

	switch(type) {
	case 0:
		// Random
	{
		std::uniform_int_distribution<size_t> rnd_st(0, 0x3FFFFFF - 1);

		for(size_t i = 0; i < size; i++) {
			data[i] = rnd_st(rng);
		}
	}
		break;
	case 1:
	{
		// Gauss ?
		int temp;
		std::uniform_int_distribution<size_t> rnd_st(0, 0x0FFFFFF - 1);
		for(size_t i = 0; i < size; i++) {
			temp=0;
			for (int j=0;j<4;j++)
				temp += rnd_st(rng);
			temp /=4;
			data[i] = temp;
		}
	}
		break;
	case 2:
	{
		std::uniform_int_distribution<size_t> rnd_st(0, 0x3FFFFFF - 1);
		// All the same
		int temp = rnd_st(rng);
		for(size_t i = 0; i < size; i++) {
			data[i] = temp;
		}
	}
		break;
	case 3:
	{
		std::uniform_int_distribution<size_t> rnd_st(0, 0x3FFFFFF - 1);
		// Bucket
		size_t temp = 0x3FFFFFF / cpus + 1;
		size_t c=0;
		for (procs_t i=0;i<cpus;i++)
		{
			procs_t j;
			size_t k;
		  for (j=0;j<cpus;j++)
		  {
			 for(k=0;k<size/cpus/cpus;k++)
			 {
			 int t2 = j * temp;
			 data[c] = t2 + ((rnd_st(rng)) / cpus);
			 c++;
			 }
		  }
		}

		for(;c<size;c++)
		{
			data[c]=rnd_st(rng);
		}
	}
		break;
	case 4:
	{
		std::uniform_int_distribution<size_t> rnd_st(0, 0x3FFFFFF - 1);
		// Staggered
		size_t c=0, temp;
		for(procs_t i=0;i<cpus;i++)
		{
			size_t j;
			if (i < (cpus/2))
				temp = 2*cpus + 1;
			else
					temp = (i - (cpus/2))*2;

			temp = temp*((0x3FFFFFF / cpus) + 1);

			for (j=0;j<size/cpus;j++)
			{
			  data[c] = temp + ((rnd_st(rng))/cpus);
			  c++;
			}
		}
		for(;c<size;c++)
		{
			data[c]=rnd_st(rng);
		}
	}
	break;
	case 5:
	{
		std::uniform_int_distribution<size_t> rnd_st(0, 0x3FFF);
		size_t val = 0;
		// Presorted ascending
		for(size_t i = 0; i < size; i++) {
			val += rnd_st(rng);
			data[i] = val;
		}
	}
	break;
	case 6:
	{
		std::uniform_int_distribution<size_t> rnd_st(0, 0x3FFF);
		size_t val = std::numeric_limits<size_t>::max();
		// Presorted descending
		for(size_t i = 0; i < size; i++) {
			val -= rnd_st(rng);
			data[i] = val;
		}
	}
	break;
	default:
		throw "unknown type for sorter";
	}

	return data;
}

template <class Pheet, template <class P> class Sorter>
size_t SortingTest<Pheet, Sorter>::get_number_of_runs(unsigned int* data) {
	unsigned int prev = data[0];
	size_t runs = 1;
//	cout << data[0];
	for(size_t i = 1; i < size; i++) {
		unsigned int current = data[i];
//		cout << "\t" << current;
		if(current < prev) {
		//	cout << "new run starts at " << i << " -2: " << data[i-2] << " -1: " << data[i-1] << " 0: " << data[i] << " 1: " << data[i+1] << " 2: " << data[i+2] << endl;
			runs++;
		}
		prev = current;
	}
	return runs;
}

}

#endif /* SORTINGTEST_H_ */
