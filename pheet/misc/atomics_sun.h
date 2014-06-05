/*
 * atomics_sun.h
 *
 *  Created on: 28.03.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ATOMICS_SUN_H_
#define ATOMICS_SUN_H_

#include <sys/types.h>
#include <sys/processor.h>
#include <sys/procset.h>
#include <atomic.h>

int
solaris_fls(long mask);

#define PTR_CAS(p, old_v, new_v)	(atomic_cas_ptr(p, old_v, new_v) == old_v)
#define INT_CAS(p, old_v, new_v)	(atomic_cas_uint((unsigned*)p, old_v, new_v) == old_v)
#define INT32_CAS(p, old_v, new_v)	(atomic_cas_32((unsigned*)p, old_v, new_v) == old_v)
#define INT64_CAS(p, old_v, new_v)	(atomic_cas_64((unsigned*)p, old_v, new_v) == old_v)
#define UINT_CAS(p, old_v, new_v)	(atomic_cas_uint(p, old_v, new_v) == old_v)
#define LONG_CAS(p, old_v, new_v)	(atomic_cas_ulong((unsigned*)p, old_v, new_v) == old_v)
#define ULONG_CAS(p, old_v, new_v)	(atomic_cas_ulong(p, old_v, new_v) == old_v)


#define INT_ATOMIC_ADD(p, val)		(atomic_add_int((unsigned*)p, val))
#define UINT_ATOMIC_ADD(p, val)		(atomic_add_int(p, val))
#define LONG_ATOMIC_ADD(p, val)		(atomic_add_long((unsigned*)p, val))
#define ULONG_ATOMIC_ADD(p, val)	(atomic_add_long(p, val))

#define INT_FETCH_AND_ADD(p, val)		(((signed)atomic_add_int_nv((unsigned*)p, val)) - val)
#define UINT_FETCH_AND_ADD(p, val)		(atomic_add_int_nv(p, val) - val)
#define LONG_FETCH_AND_ADD(p, val)		(((signed)atomic_add_long_nv((unsigned*)p, val)) - val)
#define ULONG_FETCH_AND_ADD(p, val)		(atomic_add_long_nv(p, val) - val)

#define INT_ATOMIC_SUB(p, val)		(atomic_add_int((unsigned*)p, -val))
#define UINT_ATOMIC_SUB(p, val)		(atomic_add_int(p, -val))
#define LONG_ATOMIC_SUB(p, val)		(atomic_add_long((unsigned*)p, -val))
#define ULONG_ATOMIC_SUB(p, val)	(atomic_add_long(p, -val))

#define INT_FETCH_AND_SUB(p, val)		(((signed)atomic_add_int_nv((unsigned*)p, -val)) + val)
#define UINT_FETCH_AND_SUB(p, val)		(atomic_add_int_nv(p, -val) + val)
#define LONG_FETCH_AND_SUB(p, val)		(((signed)atomic_add_long_nv((unsigned*)p, -val)) + val)
#define ULONG_FETCH_AND_SUB(p, val)		(atomic_add_long_nv(p, -val) + val)

#define MEMORY_FENCE()				membar_enter(); membar_exit()

#endif /* ATOMICS_SUN_H_ */
