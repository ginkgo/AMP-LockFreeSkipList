/*
 * CountTest.h
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef COUNTTEST_H_
#define COUNTTEST_H_

#include "../Test.h"

namespace pheet {

template <class Pheet, template <class, typename> class CountT, template <class, class> class Benchmark>
class CountTest : Test {
public:
	typedef CountT<Pheet, size_t> Count;

	CountTest(procs_t cpus, size_t blocks, double p, unsigned int seed)
	:cpus(cpus), blocks(blocks), p(p), seed(seed) {}
	~CountTest() {}

	void run_test();

private:
	procs_t cpus;
	size_t blocks;
	double p;
	unsigned int seed;
};

template <class Pheet, template <class, typename> class Count, template <class, class> class Benchmark>
void CountTest<Pheet, Count, Benchmark>::run_test() {

	typename Pheet::Environment::PerformanceCounters pc;
//	typename Partitioner<Pheet>::PerformanceCounters ppc;

	Count s;

	Time start, end;
	{typename Pheet::Environment env(cpus, pc);
		check_time(start);

		Pheet::template
			finish<Benchmark<Pheet, Count> >(s, seed, blocks, p);
		check_time(end);
	}

	size_t sum = s.get_sum();
	double seconds = calculate_seconds(start, end);
	std::cout << "test\tcounter\tscheduler\tblocks\tp\tseed\tcpus\ttotal_time\tsum\t";
	Pheet::Environment::PerformanceCounters::print_headers();
	std::cout << std::endl;
	std::cout << "count_bench\t";
	Count::print_name();
	std::cout << "\t";
	Pheet::Environment::print_name();
	std::cout << "\t" << blocks << "\t" << p << "\t" << seed << "\t" << cpus << "\t" << seconds << "\t" << sum << "\t";
	pc.print_values();
	std::cout << std::endl;
}

} /* namespace pheet */
#endif /* COUNTTEST_H_ */
