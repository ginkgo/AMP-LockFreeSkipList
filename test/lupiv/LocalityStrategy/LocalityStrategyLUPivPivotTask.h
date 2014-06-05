/*
 * LocalityStrategyLUPivPivotTask.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LOCALITYSTRATEGYLUPIVPIVOTTASK_H_
#define LOCALITYSTRATEGYLUPIVPIVOTTASK_H_

#include <pheet/pheet.h>

extern "C" {
void dswap_(int* n, double* sx, int* incx, double* sy, int* incy);
}

namespace pheet {

template <class Pheet>
class LocalityStrategyLUPivPivotTask : public Pheet::Task {
public:
	LocalityStrategyLUPivPivotTask(typename Pheet::Place** owner_info, double* a, int* pivot, int m, int lda, int n/*, PPoPPLUPivPerformanceCounters<Pheet>& pc*/);
	virtual ~LocalityStrategyLUPivPivotTask();

	virtual void operator()();

private:
	typename Pheet::Place** owner_info;
	double* a;
	int* pivot;
	int m;
	int lda;
	int n;
//	PPoPPLUPivPerformanceCounters<Pheet> pc;
};

template <class Pheet>
LocalityStrategyLUPivPivotTask<Pheet>::LocalityStrategyLUPivPivotTask(typename Pheet::Place** owner_info, double* a, int* pivot, int m, int lda, int n/*, PPoPPLUPivPerformanceCounters<Pheet>& pc*/)
: owner_info(owner_info), a(a), pivot(pivot), m(m), lda(lda), n(n)/*,pc(pc)*/ {

}

template <class Pheet>
LocalityStrategyLUPivPivotTask<Pheet>::~LocalityStrategyLUPivPivotTask() {

}

template <class Pheet>
void LocalityStrategyLUPivPivotTask<Pheet>::operator()() {
//	pc.total_tasks.incr();
//	pc.total_distance_to_last_location.add(Pheet::get_place()->get_distance(*owner_info));

/*	if(*owner_info==Pheet::get_place())
		pc.slices_rescheduled_at_same_place.incr();*/

	(*owner_info) = Pheet::get_place();

	for(int j = 0; j < m; ++j) {
		if(pivot[j] != j + 1) {
			dswap_(&n,a + j, &lda, a + pivot[j] - 1, &lda);
		}
	}
}

}

#endif /* LOCALITYSTRATEGYLUPIVPIVOTTASK_H_ */
