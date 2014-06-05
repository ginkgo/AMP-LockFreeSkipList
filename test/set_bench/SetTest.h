/*
 * SetTest.h
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef SETTEST_H_
#define SETTEST_H_

#include "SetBenchTask.h"
#include "../Test.h"

namespace pheet {

template <class Pheet, template <class, typename, class> class SetT>
class SetTest : Test {
public:
	typedef SetT<Pheet, size_t, std::less<size_t>> Set;

	SetTest(procs_t cpus, size_t range, size_t blocks, double contains_p, double add_p, unsigned int seed)
	:cpus(cpus), range(range), blocks(blocks), contains_p(contains_p), add_p(add_p), seed(seed) {}
	~SetTest() {}

	void run_test();

private:
	procs_t cpus;
	size_t range;
	size_t blocks;
	double contains_p;
	double add_p;
	unsigned int seed;
};

template <class Pheet, template <class, typename, class> class Set>
void SetTest<Pheet, Set>::run_test() {

	typename Pheet::Environment::PerformanceCounters pc;
//	typename Partitioner<Pheet>::PerformanceCounters ppc;

	Time start, end;
	{typename Pheet::Environment env(cpus, pc);
		Set s;
		check_time(start);

		Pheet::template
			finish<SetBenchTask<Pheet, Set> >(s, range, seed, blocks, contains_p, add_p);
		check_time(end);
	}

	double seconds = calculate_seconds(start, end);
	std::cout << "test\tset\tscheduler\trange\tblocks\tcontains_p\tadd_p\tseed\tcpus\ttotal_time\t";
	Pheet::Environment::PerformanceCounters::print_headers();
	std::cout << std::endl;
	std::cout << "set_bench\t";
	Set::print_name();
	std::cout << "\t";
	Pheet::Environment::print_name();
	std::cout << "\t" << range << "\t" << blocks << "\t" << contains_p << "\t" << add_p << "\t" << seed << "\t" << cpus << "\t" << seconds << "\t";
	pc.print_values();
	std::cout << std::endl;
}

} /* namespace pheet */
#endif /* SETTEST_H_ */
