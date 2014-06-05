/*
 * bitops_win.h
 *
 *  Created on: 13.08.2013
 *      Author: Manuel Pöter
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BITOPS_WIN_H_
#define BITOPS_WIN_H_

#include <intrin.h>

static inline int
find_first_bit_set(unsigned int x)
{
	unsigned long index;
	if (_BitScanForward(&index, x) == 0)
		return 0;
	return index + 1;
}

static inline int
find_last_bit_set(unsigned int x)
{
	unsigned long index;
	if (_BitScanReverse(&index, x) == 0)
		return 0;
	return index + 1;
}

#endif /* BITOPS_WIN_H_ */
