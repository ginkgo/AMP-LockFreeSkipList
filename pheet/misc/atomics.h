/*
 * atomics.h
 *
 *  Created on: 28.03.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ATOMICS_H_
#define ATOMICS_H_

#include "../settings.h"

#ifdef __ICC
#include "atomics_icc.h"

#elif ENV_WINDOWS

#include "atomics_c11.h"

#elif ENV_GCC
// Working with g++

#include "atomics_gnu.h"

#elif ENV_CLANG

#include "atomics_clang.h"

#elif ENV_SOLARIS_SUNCC

#include "atomics_sun.h"

#else
#error "Unsupported environment or environment not recognized"
#endif

#endif /* ATOMICS_H_ */
