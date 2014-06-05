/*
 * LUPivMMTask.h
 *
 *  Created on: Dec 20, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LUPIVMMTASK_H_
#define LUPIVMMTASK_H_

#include <pheet/pheet.h>

extern "C" {
void dgemm_(char * transA, char* transB, int* m, int* n, int* k, double* alpha, double* a, int* lda, double* b, int* ldb, double* beta, double* c, int* ldc);
}

namespace pheet {

template <class Pheet>
class LUPivMMTask : public Pheet::Task {
public:
	LUPivMMTask(double* a, double* b, double* c, int k, int lda);
	virtual ~LUPivMMTask();

	virtual void operator()();

private:
	double* a;
	double* b;
	double* c;
	int k;
	int lda;
};

template <class Pheet>
LUPivMMTask<Pheet>::LUPivMMTask(double* a, double* b, double* c, int k, int lda)
: a(a), b(b), c(c), k(k), lda(lda) {

}

template <class Pheet>
LUPivMMTask<Pheet>::~LUPivMMTask() {

}

template <class Pheet>
void LUPivMMTask<Pheet>::operator()() {
	char trans_a = 'n';
	char trans_b = 'n';
	double alpha = -1.0;
	double beta = 1.0;
	dgemm_(&trans_a, &trans_b, &k, &k, &k, &alpha, a, &lda, b, &lda, &beta, c, &lda);
}

}

#endif /* LUPIVMMTASK_H_ */
