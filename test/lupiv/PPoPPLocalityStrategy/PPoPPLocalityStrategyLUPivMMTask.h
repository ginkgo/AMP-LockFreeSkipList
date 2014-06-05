/*
 * PPoPPLocalityStrategyLUPivMMTask.h
 *
 *  Created on: Aug 2, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef PPOPPLOCALITYSTRATEGYLUPIVMMTASK_H_
#define PPOPPLOCALITYSTRATEGYLUPIVMMTASK_H_

#include "PPoPPLocalityStrategyLUPivPerformanceCounters.h"

#include <pheet/pheet.h>

extern "C" {
void dgemm_(char * transA, char* transB, int* m, int* n, int* k, double* alpha, double* a, int* lda, double* b, int* ldb, double* beta, double* c, int* ldc);
}

namespace pheet {

template <class Pheet>
class PPoPPLocalityStrategyLUPivMMTask : public Pheet::Task {
public:
	typedef PPoPPLocalityStrategyLUPivPerformanceCounters<Pheet> PerformanceCounters;

	PPoPPLocalityStrategyLUPivMMTask(typename Pheet::Place** owner_info, double* a, double* b, double* c, int k, int lda, PerformanceCounters& pc);
	virtual ~PPoPPLocalityStrategyLUPivMMTask();

	virtual void operator()();

private:
	typename Pheet::Place** owner_info;
	double* a;
	double* b;
	double* c;
	int k;
	int lda;

	PerformanceCounters pc;
};

template <class Pheet>
PPoPPLocalityStrategyLUPivMMTask<Pheet>::PPoPPLocalityStrategyLUPivMMTask(typename Pheet::Place** owner_info, double* a, double* b, double* c, int k, int lda, PerformanceCounters& pc)
: owner_info(owner_info), a(a), b(b), c(c), k(k), lda(lda), pc(pc) {

}

template <class Pheet>
PPoPPLocalityStrategyLUPivMMTask<Pheet>::~PPoPPLocalityStrategyLUPivMMTask() {

}

template <class Pheet>
void PPoPPLocalityStrategyLUPivMMTask<Pheet>::operator()() {
	auto place = Pheet::get_place();
	pc.total_tasks.incr();
	pc.locality.add(typename PerformanceCounters::LocalityInfo(false, (*owner_info)->get_id(), place->get_id()));
	if(*owner_info != place) {
		pc.locality_misses.incr();
		pc.total_distance.add(place->get_distance(*owner_info));
	}
	(*owner_info) = place;

	char trans_a = 'n';
	char trans_b = 'n';
	double alpha = -1.0;
	double beta = 1.0;
	dgemm_(&trans_a, &trans_b, &k, &k, &k, &alpha, a, &lda, b, &lda, &beta, c, &lda);
}

} /* namespace pheet */
#endif /* PPOPPLOCALITYSTRATEGYLUPIVMMTASK_H_ */
