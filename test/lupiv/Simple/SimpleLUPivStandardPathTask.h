/*
 * SimpleLUPivStandardPathTask.h
 *
 *  Created on: Dec 20, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SIMPLELUPIVSTANDARDPATHTASK_H_
#define SIMPLELUPIVSTANDARDPATHTASK_H_

#include "../helpers/LUPivPivotTask.h"
#include "../helpers/LUPivMMTask.h"

#include <algorithm>

extern "C" {
void dtrsm_(char *side, char *uplo, char* transa, char* diag, int *m, int *n, double* alpha, double* a, int* lda, double* b, int* ldb);
}

namespace pheet {

template <class Pheet, int BLOCK_SIZE>
class SimpleLUPivStandardPathTask : public Pheet::Task {
public:
	SimpleLUPivStandardPathTask(double* a, double* lu_col, int* pivot, int m, int lda, int n);
	virtual ~SimpleLUPivStandardPathTask();

	virtual void operator()();

private:
	double* a;
	double* lu_col;
	int* pivot;
	int m;
	int lda;
	int n;
};

template <class Pheet, int BLOCK_SIZE>
SimpleLUPivStandardPathTask<Pheet, BLOCK_SIZE>::SimpleLUPivStandardPathTask(double* a, double* lu_col, int* pivot, int m, int lda, int n)
: a(a), lu_col(lu_col), pivot(pivot), m(m), lda(lda), n(n) {

}

template <class Pheet, int BLOCK_SIZE>
SimpleLUPivStandardPathTask<Pheet, BLOCK_SIZE>::~SimpleLUPivStandardPathTask() {

}

template <class Pheet, int BLOCK_SIZE>
void SimpleLUPivStandardPathTask<Pheet, BLOCK_SIZE>::operator()() {
	pheet_assert(n <= BLOCK_SIZE);

	// Apply pivot to column
	Pheet::template
		call<LUPivPivotTask<Pheet> >(a, pivot, std::min(m, BLOCK_SIZE), lda, n);

	int k = std::min(std::min(m, n), BLOCK_SIZE);

	{	// Apply TRSM to uppermost block
		char side = 'l';
		char uplo = 'l';
		char transa = 'n';
		char diag = 'u';
		double alpha = 1.0;
		dtrsm_(&side, &uplo, &transa, &diag, &k, &k, &alpha, lu_col, &lda, a, &lda);
	}

	// Apply matrix multiplication to all other blocks
	int num_blocks = m / BLOCK_SIZE;

	for(int i = 1; i < num_blocks; ++i) {
		int pos = i * BLOCK_SIZE;
		Pheet::template
			spawn<LUPivMMTask<Pheet> >(lu_col + pos, a, a + pos, k, lda);
	}
}

}

#endif /* SIMPLELUPIVSTANDARDPATHTASK_H_ */
