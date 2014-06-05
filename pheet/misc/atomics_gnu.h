/*
 * atomics_gnu.h
 *
 *  Created on: 28.03.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ATOMICS_GNU_H_
#define ATOMICS_GNU_H_

#define PTR_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define INT_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define INT32_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define INT64_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define UINT64_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define UINT_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define LONG_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define ULONG_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define SIZET_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define PTRDIFFT_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))

#define INT_ATOMIC_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define UINT_ATOMIC_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define LONG_ATOMIC_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define ULONG_ATOMIC_ADD(p, val)	(__sync_fetch_and_add(p, val))
#define SIZET_ATOMIC_ADD(p, val)	(__sync_fetch_and_add(p, val))

#define INT_FETCH_AND_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define UINT_FETCH_AND_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define LONG_FETCH_AND_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define ULONG_FETCH_AND_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define SIZET_FETCH_AND_ADD(p, val)		(__sync_fetch_and_add(p, val))

#define INT_ATOMIC_SUB(p, val)		(__sync_fetch_and_sub(p, val))
#define UINT_ATOMIC_SUB(p, val)		(__sync_fetch_and_sub(p, val))
#define LONG_ATOMIC_SUB(p, val)		(__sync_fetch_and_sub(p, val))
#define ULONG_ATOMIC_SUB(p, val)	(__sync_fetch_and_sub(p, val))
#define SIZET_ATOMIC_SUB(p, val)	(__sync_fetch_and_sub(p, val))
#define PTRDIFFT_ATOMIC_SUB(p, val)	(__sync_fetch_and_sub(p, val))

#define INT_FETCH_AND_SUB(p, val)		(__sync_fetch_and_sub(p, val))
#define UINT_FETCH_AND_SUB(p, val)		(__sync_fetch_and_sub(p, val))
#define LONG_FETCH_AND_SUB(p, val)		(__sync_fetch_and_sub(p, val))
#define ULONG_FETCH_AND_SUB(p, val)		(__sync_fetch_and_sub(p, val))
inline size_t size_t_fetch_and_sub(size_t* p, size_t val) {
	return __sync_fetch_and_sub(p, val);
}
#define SIZET_FETCH_AND_SUB(p, val)		(size_t_fetch_and_sub(p, val))

#define MEMORY_FENCE()				(__sync_synchronize())

#endif /* ATOMICS_GNU_H_ */
