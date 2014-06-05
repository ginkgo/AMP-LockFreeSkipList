/*
 * SchedulerTask.h
 *
 *  Created on: 25.07.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SCHEDULERTASK_H_
#define SCHEDULERTASK_H_

/*
 *
 */
namespace pheet {

template <class Pheet>
class SchedulerTask {
public:
	SchedulerTask();
	virtual ~SchedulerTask();

	virtual void operator()() = 0;

private:
};

template <class Sched>
SchedulerTask<Sched>::SchedulerTask() {

}

template <class Sched>
SchedulerTask<Sched>::~SchedulerTask() {

}

}

#endif /* SCHEDULERTASK_H_ */
