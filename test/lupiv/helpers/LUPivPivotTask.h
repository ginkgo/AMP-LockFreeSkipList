/*
 * LUPivPivotTask.h
 *
 *  Created on: Dec 20, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LUPIVPIVOTTASK_H_
#define LUPIVPIVOTTASK_H_

#include <pheet/pheet.h>

extern "C" {
void dswap_(int* n, double* sx, int* incx, double* sy, int* incy);
}

namespace pheet {

template <class Pheet>
class LUPivPivotTask : public Pheet::Task {
public:
	LUPivPivotTask(double* a, int* pivot, int m, int lda, int n/*, PPoPPLUPivPerformanceCounters<Pheet>& pc*/);
	virtual ~LUPivPivotTask();

	virtual void operator()();

private:
	double* a;
	int* pivot;
	int m;
	int lda;
	int n;
};

template <class Pheet>
LUPivPivotTask<Pheet>::LUPivPivotTask(double* a, int* pivot, int m, int lda, int n/*,PPoPPLUPivPerformanceCounters<Pheet>& pc*/)
: a(a), pivot(pivot), m(m), lda(lda), n(n) {

}

template <class Pheet>
LUPivPivotTask<Pheet>::~LUPivPivotTask() {

}

template <class Pheet>
void LUPivPivotTask<Pheet>::operator()() {
	for(int j = 0; j < m; ++j) {
		if(pivot[j] != j + 1) {
			dswap_(&n,a + j, &lda, a + pivot[j] - 1, &lda);
		}
	}
}

}

#endif /* LUPIVPIVOTTASK_H_ */
