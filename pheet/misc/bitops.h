/*
 * bitops.h
 *
 *  Created on: 28.03.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BITOPS_H_
#define BITOPS_H_

namespace pheet {

#if ENV_WINDOWS

#include "bitops_win.h"

#else

static inline int
find_first_bit_set(unsigned int x)
{
	return ffs(x);
}

static inline int
find_first_bit_set(unsigned long int x)
{
	return ffsl(x);
}

#ifdef ENV_X86

#include "bitops_x86.h"

#else

static inline int
find_last_bit_set(unsigned long int x)
{
	unsigned int result = 0;
	while (x != 0) {
		++result;
		x >>= 1;
	}
	return result;
}

static inline int
find_last_bit_set(unsigned int x)
{
	unsigned int result = 0;
	while (x != 0) {
		++result;
		x >>= 1;
	}
	return result;
}

#endif

#endif

}

#endif /* BITOPS_H_ */
