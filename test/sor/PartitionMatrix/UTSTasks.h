/*
* InARow.h
*
*  Created on: 22.09.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef UTSTASKA_H_
#define UTSTASKA_H_

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
	class UTSStartTaskStrat : public Pheet::Task
	{
		Node parent;
		bool usestrat;
	public:

	UTSStartTaskStrat(Node parent, bool usestrat):parent(parent),usestrat(usestrat) { }
		virtual ~UTSStartTaskStrat() {}

		void operator()() //(typename Task::TEC& tec)
		{
			Node child;
			int parentHeight = parent.height;
			int numChildren, childType;

			numChildren = uts_numChildren(&parent);
			childType   = uts_childType(&parent);

			// record number of children in parent
			parent.numChildren = numChildren;
  
			// construct children and push onto stack
			if (numChildren > 0) 
			{
				typename Pheet::Finish f;
				int i, j;
				child.type = childType;
				child.height = parentHeight + 1;

			    for (i = 0; i < numChildren; i++) 
				{
					for (j = 0; j < computeGranularity; j++) 
					{
					  // printf(".");
						// TBD:  add parent height to spawn
						// computeGranularity controls number of rng_spawn calls per node
						rng_spawn(parent.state.state, child.state.state, i);
						if(usestrat)
						  Pheet::template spawn_s<UTSStartTaskStrat<Pheet> >(UTSStrategy<Pheet>(parentHeight, Pheet::get_place()),child,usestrat);
						else
						  Pheet::template spawn<UTSStartTaskStrat<Pheet> >(child,usestrat);
					}
				}
			}
		}
	};
}
#endif /* INAROWTASK_H_ */
