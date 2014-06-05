/*
 * FinisherSchedulerTask.h
 *
 *  Created on: May 23, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FINISHERSCHEDULERTASK_H_
#define FINISHERSCHEDULERTASK_H_

#include "../common/SchedulerTask.h"

namespace pheet {

template <class Pheet>
class FinisherSchedulerTask : public SchedulerTask<Pheet> {
public:
	typename Pheet::Finisher fin;
};

} /* namespace pheet */
#endif /* FINISHERSCHEDULERTASK_H_ */
