/*
 * Strategy2UTS.h
 *
 *  Created on: 16.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGY2UTS_H_
#define STRATEGY2UTS_H_

#include "Strategy2UTSTask.h"

namespace pheet {

template <class Pheet>
class Strategy2UTS {
public:
	Strategy2UTS(procs_t cpus)
	:cpus(cpus) {}
	~Strategy2UTS() {}

	void run() {
		Node root;
		uts_initRoot(&root, type);
		typename Pheet::Environment env(cpus,pc);
		Pheet::template finish<Strategy2UTSTask<Pheet> >(root);
	}

	void print_results() {
		pc.print_values();
	}

	void print_headers() {
		Pheet::Environment::PerformanceCounters::print_headers();
	}

	static void print_name() {
		std::cout << name;
	}
	static void print_scheduler_name() {
		Pheet::Environment::print_name();
	}

	static char const name[];
private:
    procs_t cpus;
	typename Pheet::Environment::PerformanceCounters pc;
};

template <class Pheet>
char const Strategy2UTS<Pheet>::name[] = "Strategy2UTS";

} /* namespace pheet */
#endif /* STRATEGY2UTS_H_ */
