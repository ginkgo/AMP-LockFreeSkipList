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

#undef max
#undef min

#include "UTSTasks.h"

using namespace std;

namespace pheet {
  
  template <class Pheet>
    class UTSStartTask;
  
  
  template <class Pheet>
    class SORAndUTSTask : public Pheet::Task
    {
      
      SORParams& sp;
      int iterations;
      SORPerformanceCounters<Pheet> pc;
      bool runuts;
      bool runsor;
      bool usestrat;
    public:
      
    SORAndUTSTask(SORParams& sp, int iterations, SORPerformanceCounters<Pheet>& pc, bool runuts, bool runsor, bool usestrat):sp(sp),iterations(iterations),pc(pc),runuts(runuts),runsor(runsor),usestrat(usestrat) { }
      virtual ~SORAndUTSTask() {}
      
      void operator()()
      {
	typename Pheet::Finish f;
	Node root;
        uts_initRoot(&root, type);

	if(runuts)
	  Pheet::template spawn<UTSStartTaskStrat<Pheet> >(root,usestrat);
	if(runsor)
	  Pheet::template spawn<SORStartTask<Pheet> >(sp, iterations,pc,usestrat);
      }

    };
}
#endif /* INAROWTASK_H_ */
