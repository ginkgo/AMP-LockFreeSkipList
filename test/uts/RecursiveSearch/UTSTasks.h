/*
* InARow.h
*
*  Created on: 22.09.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef UTSTASK_H_
#define UTSTASK_H_

#include <pheet/pheet.h>
#include "../../../pheet/misc/types.h"
#include "../../../pheet/misc/atomics.h"
//#include "../../test_schedulers.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <exception>

#include "uts.h"

using namespace std;

namespace pheet {

	template <class Pheet>
	class UTSStartTask : public Pheet::Task
	{
		Node parent;
	public:

		UTSStartTask(Node parent):parent(parent) { }
		virtual ~UTSStartTask() {}

		void operator()()
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
					Pheet::template spawn<UTSStartTask<Pheet> >(child);
				}
			}
		}
	};
}
#endif /* INAROWTASK_H_ */
