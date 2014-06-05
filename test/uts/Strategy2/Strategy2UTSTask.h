/*
 * Strategy2UTSTask.h
 *
 *  Created on: 16.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGY2UTSTASK_H_
#define STRATEGY2UTSTASK_H_

#include <pheet/pheet.h>
#include "../../../pheet/misc/types.h"
#include "../../../pheet/misc/atomics.h"
//#include "../../test_schedulers.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <exception>

#include "../RecursiveSearch/uts.h"
#include "Strategy2UTSStrategy.h"


namespace pheet {

template <class Pheet>
class Strategy2UTSTask  : public Pheet::Task {
public:
	typedef Strategy2UTSTask<Pheet> Self;
	typedef Strategy2UTSStrategy<Pheet> Strategy;

	Strategy2UTSTask(Node parent)
	:parent(parent) { }
	virtual ~Strategy2UTSTask() {}

virtual void operator()()
{
	Node child;
	int parentHeight = parent.height;
	int numChildren;

	numChildren = uts_numChildren(&parent);
	auto childType   = uts_childType(&parent);

	// record number of children in parent
	parent.numChildren = numChildren;

	// construct children and push onto stack
	if (numChildren > 0)
	{
		int i, j;
		child.type = childType;
		child.height = parentHeight + 1;

		for (i = 0; i < numChildren; i++)
		{
			for (j = 0; j < computeGranularity; j++)
			{
				// computeGranularity controls number of rng_spawn calls per node
				rng_spawn(parent.state.state, child.state.state, i);
			}
			Pheet::template spawn_s<Self>(Strategy(child.height), child);
		}
	}
}

private:
	Node parent;
};

} /* namespace pheet */
#endif /* STRATEGY2UTSTASK_H_ */
