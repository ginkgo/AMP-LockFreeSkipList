/*
 * atomics_clang.h
 *
 *  Created on: May 15, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ATOMICS_CLANG_H_
#define ATOMICS_CLANG_H_

#define PTR_CAS(p, old_v, new_v)    (ptr_compare_and_swap(p, old_v, new_v))
template <typename T>
inline bool ptr_compare_and_swap(T** p, T* old_v, T* new_v) {
    return __sync_bool_compare_and_swap(p, old_v, new_v);
}
template <typename T>
inline bool ptr_compare_and_swap(T** p, std::nullptr_t old_v, T* new_v) {
    return __sync_bool_compare_and_swap(p, static_cast<T*>(old_v), new_v);
}
#define INT_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define INT32_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define INT64_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define UINT64_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define UINT_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define LONG_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
#define ULONG_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))
inline bool size_t_cas_clang(size_t* p, size_t old_v, size_t new_v) {
	return __sync_bool_compare_and_swap(p, old_v, new_v);
}

#define SIZET_CAS(p, old_v, new_v)	(size_t_cas_clang(p, old_v, new_v))
#define PTRDIFFT_CAS(p, old_v, new_v)	(__sync_bool_compare_and_swap(p, old_v, new_v))

#define INT_ATOMIC_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define UINT_ATOMIC_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define LONG_ATOMIC_ADD(p, val)		(__sync_fetch_and_add(p, val))
#define ULONG_ATOMIC_ADD(p, val)	(__sync_fetch_and_add(p, val))
inline bool size_t_atomic_add_clang(size_t* p, size_t val) {
	return __sync_fetch_and_add(p, val);
}
#define SIZET_ATOMIC_ADD(p, val)	(size_t_atomic_add_clang(p, val))

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
#define SIZET_FETCH_AND_SUB(p, val)		(__sync_fetch_and_sub(p, val))

#define MEMORY_FENCE()				(__sync_synchronize())

#endif /* ATOMICS_CLANG_H_ */
