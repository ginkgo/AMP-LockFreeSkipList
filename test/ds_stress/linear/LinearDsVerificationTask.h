/*
 * LinearDsVerificationTask.h
 *
 *  Created on: May 8, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LINEARDSVERIFICATIONTASK_H_
#define LINEARDSVERIFICATIONTASK_H_

#include <atomic>

#include "LinearDsCorrectnessTestTask.h"

namespace pheet {

struct LinearDsVerificationChoice {
	typedef LinearDsVerificationChoice Self;

	LinearDsVerificationChoice(Self* parent, size_t i, size_t depth)
	:parent(parent), i(i), depth(depth) {}

	Self* parent;
	size_t i;
	size_t depth;
};

template <class Pheet>
class LinearDsVerificationTask : public Pheet::Task {
public:
	typedef LinearDsVerificationTask<Pheet> Self;
	typedef LinearDsCorrectnessTestEvent Event;
	typedef LinearDsVerificationChoice Choice;
	typedef typename Pheet::Finish Finish;

	LinearDsVerificationTask(Event* history, size_t n, procs_t cpus)
	: history(history), n(n), cpus(cpus), parent_choice(nullptr), i(0) {

	}

	LinearDsVerificationTask(Event* history, size_t n, procs_t cpus, Choice* parent, size_t i)
	: history(history), n(n), cpus(cpus), parent_choice(parent), i(i) {

	}

	~LinearDsVerificationTask() {

	}

	virtual void operator()() {
		// Reconstruct and verify fixed state (+ calculate rho)


		// Lower bound all values. Terminate if no improvement can be expected from this branch


		// spawn sub-tasks (use finisher hyperobject to ensure choice gets deleted (Finish region could be problematic, due to stack depth))
	}


private:
	Event* history;
	size_t n;
	procs_t cpus;

	Choice* parent_choice;
	size_t i;
/*	Self* parent;
	size_t i;
	size_t depth;*/
};

} /* namespace pheet */
#endif /* LINEARDSVERIFICATIONTASK_H_ */
