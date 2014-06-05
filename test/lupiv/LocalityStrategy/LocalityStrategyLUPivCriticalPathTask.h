/*
 * LocalityStrategyLUPivCriticalPathTask.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LOCALITYSTRATEGYLUPIVCRITICALPATHTASK_H_
#define LOCALITYSTRATEGYLUPIVCRITICALPATHTASK_H_

#include "../helpers/LUPivPivotTask.h"
#include "LocalityStrategyLUPivMMTask.h"
#include "LUPivLocalityStrategy.h"

#include <algorithm>

extern "C" {
void dtrsm_(char *side, char *uplo, char* transa, char* diag, int *m, int *n, double* alpha, double* a, int* lda, double* b, int* ldb);
void dgetf2_(int *m, int *n, double *a, int *lda, int *piv, int *info);
}

namespace pheet {

template <class Pheet, int BLOCK_SIZE>
class LocalityStrategyLUPivCriticalPathTask : public Pheet::Task {
public:
	LocalityStrategyLUPivCriticalPathTask(typename Pheet::Place** owner_info, typename Pheet::Place** block_owners, double* a, double* lu_col, int* pivot, int m, int lda, int n);
	virtual ~LocalityStrategyLUPivCriticalPathTask();

	virtual void operator()();

private:
	typename Pheet::Place** owner_info;
	typename Pheet::Place** block_owners;
	double* a;
	double* lu_col;
	int* pivot;
	int m;
	int lda;
	int n;
};

template <class Pheet, int BLOCK_SIZE>
LocalityStrategyLUPivCriticalPathTask<Pheet, BLOCK_SIZE>::LocalityStrategyLUPivCriticalPathTask(typename Pheet::Place** owner_info, typename Pheet::Place** block_owners, double* a, double* lu_col, int* pivot, int m, int lda, int n)
: owner_info(owner_info), block_owners(block_owners), a(a), lu_col(lu_col), pivot(pivot), m(m), lda(lda), n(n) {

}

template <class Pheet, int BLOCK_SIZE>
LocalityStrategyLUPivCriticalPathTask<Pheet, BLOCK_SIZE>::~LocalityStrategyLUPivCriticalPathTask() {

}

template <class Pheet, int BLOCK_SIZE>
void LocalityStrategyLUPivCriticalPathTask<Pheet, BLOCK_SIZE>::operator()() {
	(*owner_info) = Pheet::get_place();

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

	int num_blocks = m / BLOCK_SIZE;

	{typename Pheet::Finish f;
		// Apply matrix multiplication to all other blocks
		for(int i = 1; i < num_blocks; ++i) {
			int pos = i * BLOCK_SIZE;
			Pheet::template
				spawn_prio<LocalityStrategyLUPivMMTask<Pheet> >(
						LUPivLocalityStrategy<Pheet>(block_owners[i], 5, 3),
						block_owners + i,
						lu_col + pos, a, a + pos, k, lda);
		}
	}

	if(num_blocks > 1) {
		// Apply sequential algorithm to the rest of the column
		int out = 0;
		int tmp_m = m - BLOCK_SIZE;
		int tmp_n = std::min(BLOCK_SIZE, n);
		dgetf2_(&tmp_m, &tmp_n, a + BLOCK_SIZE, &lda, pivot + BLOCK_SIZE, &out);
	}
}

}

#endif /* LOCALITYSTRATEGYLUPIVCRITICALPATHTASK_H_ */
