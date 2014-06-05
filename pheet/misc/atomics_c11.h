/*
 * atomics_c11.h
 *
 *  Created on: 13.08.203
 *      Author: Manuel Pöter
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ATOMICS_C11_H_
#define ATOMICS_C11_H_

#include <atomic>
#include <type_traits>

template <typename T, typename U, typename V>
inline bool generic_cas(T* p, U old_v, V new_v)
{
	T local_old = old_v;
	return std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic<T>*>(p), &local_old, static_cast<T>(new_v));
}
#define PTR_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define INT_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define INT32_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define INT64_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define UINT64_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define UINT_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define LONG_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define ULONG_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define SIZET_CAS(p, old_v, new_v)		(generic_cas(p, old_v, new_v))
#define PTRDIFFT_CAS(p, old_v, new_v)	(generic_cas(p, old_v, new_v))

template <typename T, typename U>
inline T atomic_add(T* p, U val)
{
	typedef std::remove_volatile<T>::type type;
	return std::atomic_fetch_add<type>(reinterpret_cast<volatile std::atomic<type>*>(p), static_cast<type>(val));
}
#define INT_ATOMIC_ADD(p, val)		(atomic_add(p, val))
#define UINT_ATOMIC_ADD(p, val)		(atomic_add(p, val))
#define LONG_ATOMIC_ADD(p, val)		(atomic_add(p, val))
#define ULONG_ATOMIC_ADD(p, val)	(atomic_add(p, val))
#define SIZET_ATOMIC_ADD(p, val)	(atomic_add(p, val))

#define INT_FETCH_AND_ADD(p, val)	(atomic_add(p, val))
#define UINT_FETCH_AND_ADD(p, val)	(atomic_add(p, val))
#define LONG_FETCH_AND_ADD(p, val)	(atomic_add(p, val))
#define ULONG_FETCH_AND_ADD(p, val)	(atomic_add(p, val))
#define SIZET_FETCH_AND_ADD(p, val)	(atomic_add(p, val))

template <typename T, typename U>
inline T atomic_sub(T* p, U val)
{
	typedef std::remove_volatile<T>::type type;
	return std::atomic_fetch_sub<type>(reinterpret_cast<volatile std::atomic<type>*>(p), static_cast<type>(val));
}
#define INT_ATOMIC_SUB(p, val)		(atomic_sub(p, val))
#define UINT_ATOMIC_SUB(p, val)		(atomic_sub(p, val))
#define LONG_ATOMIC_SUB(p, val)		(atomic_sub(p, val))
#define ULONG_ATOMIC_SUB(p, val)	(atomic_sub(p, val))
#define SIZET_ATOMIC_SUB(p, val)	(atomic_sub(p, val))
#define PTRDIFFT_ATOMIC_SUB(p, val)	(atomic_sub(p, val))

#define INT_FETCH_AND_SUB(p, val)	(atomic_sub(p, val))
#define UINT_FETCH_AND_SUB(p, val)	(atomic_sub(p, val))
#define LONG_FETCH_AND_SUB(p, val)	(atomic_sub(p, val))
#define ULONG_FETCH_AND_SUB(p, val)	(atomic_sub(p, val))
#define SIZET_FETCH_AND_SUB(p, val)	(atomic_sub(p, val))

#define MEMORY_FENCE()				(std::atomic_thread_fence(std::memory_order_seq_cst))

#endif /* ATOMICS_C11_H_ */
