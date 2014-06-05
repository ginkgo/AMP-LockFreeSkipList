/*
 * LocalityStrategyLUPivMMTask.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LOCALITYSTRATEGYLUPIVMMTASK_H_
#define LOCALITYSTRATEGYLUPIVMMTASK_H_

#include <pheet/pheet.h>

extern "C" {
void dgemm_(char * transA, char* transB, int* m, int* n, int* k, double* alpha, double* a, int* lda, double* b, int* ldb, double* beta, double* c, int* ldc);
}

namespace pheet {

template <class Pheet>
class LocalityStrategyLUPivMMTask : public Pheet::Task {
public:
	LocalityStrategyLUPivMMTask(typename Pheet::Place** owner_info, double* a, double* b, double* c, int k, int lda/*, PPoPPLUPivPerformanceCounters<Pheet>& pc*/);
	virtual ~LocalityStrategyLUPivMMTask();

	virtual void operator()();

private:
	typename Pheet::Place** owner_info;
	double* a;
	double* b;
	double* c;
	int k;
	int lda;
//	PPoPPLUPivPerformanceCounters<Pheet> pc;

};

template <class Pheet>
LocalityStrategyLUPivMMTask<Pheet>::LocalityStrategyLUPivMMTask(typename Pheet::Place** owner_info, double* a, double* b, double* c, int k, int lda/*, PPoPPLUPivPerformanceCounters<Pheet>& pc*/)
: owner_info(owner_info), a(a), b(b), c(c), k(k), lda(lda)/*, pc(pc)*/ {

}

template <class Pheet>
LocalityStrategyLUPivMMTask<Pheet>::~LocalityStrategyLUPivMMTask() {

}

template <class Pheet>
void LocalityStrategyLUPivMMTask<Pheet>::operator()() {
/*	pc.total_tasks.incr();
	pc.total_distance_to_last_location.add(Pheet::get_place()->get_distance(*owner_info));

	if(*owner_info==Pheet::get_place())
		pc.slices_rescheduled_at_same_place.incr();
*/
	(*owner_info) = Pheet::get_place();

	char trans_a = 'n';
	char trans_b = 'n';
	double alpha = -1.0;
	double beta = 1.0;
	dgemm_(&trans_a, &trans_b, &k, &k, &k, &alpha, a, &lda, b, &lda, &beta, c, &lda);
}

}

#endif /* LOCALITYSTRATEGYLUPIVMMTASK_H_ */
