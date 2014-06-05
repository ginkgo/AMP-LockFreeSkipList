/*
 * FastBitset.h
 *
 *	Created on: Dec 23, 2012
 *	Author: Manuel Poeter
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef FASTBITSET_H_
#define FASTBITSET_H_

#if !defined(__ICC) && !defined(ENV_WINDOWS)
#include <bitset>
template <size_t SIZE>
using FastBitset = std::bitset<SIZE>;

#else
#include <immintrin.h>

namespace pheet {

template <size_t SIZE>
class FastBitset {
	static_assert(SIZE == sizeof(size_t) * 8, "the current implementation supports only bitsets of sizeof(void*)");
	static const size_t BitsPerWord = sizeof(int) * 8;
	static const size_t NumberOfWords = (SIZE - 1) / BitsPerWord + 1;

	size_t data;

#ifdef __ICC
	static inline size_t bit_scan_forward(size_t word)
	{
		asm("bsf %1,%0"
			: "=r" (word)
			: "rm" (word));
		return word;
	}
	
	static inline int test_bit(size_t nr, volatile const size_t *addr)
	{
		int oldbit;
		asm volatile("bt %2, %1\n"
			"sbb %0,%0"
			: "=r" (oldbit)
			: "m" (*(size_t *)addr), "Ir" (nr));

		return oldbit;
	}
#elif ENV_WINDOWS
	static inline size_t bit_scan_forward(size_t word)
	{
		unsigned long index = 0;;
#ifdef _M_X64
		_BitScanForward64(&index, word);
#else
		_BitScanForward(&index, word);
#endif
		return index;
	}

	static inline int test_bit(size_t nr, volatile const size_t *addr)
	{
#ifdef _M_X64
		return _bittest64(addr, nr);
#else
		return _bittest(addr, nr);
#endif
	}
#endif
public:
	FastBitset() : data() {}

	size_t size() const { return SIZE; }

	bool test(size_t bit) const 
	{
		pheet_assert(bit < SIZE);
		return (data & ((size_t)1 << bit)) != 0;
		//return (bool)test_bit(bit, &data);
	}

	void set(size_t bit, bool v = true) 
	{
		pheet_assert(bit < SIZE);
		size_t x = (size_t)1 << bit;
		if (v) 
			data |= x;
		else
			data &= ~x;
	}
 
	size_t count() const
	{
		return _mm_popcnt_u64(data);
	}

	void reset() { data = 0; }

	size_t _Find_first() const { return find_first(); }
	size_t find_first() const
	{
		if (data == 0)
			return size();
		return bit_scan_forward(data);
	}

	size_t _Find_next(size_t pos) const { return find_next(pos); }
	size_t find_next(size_t pos) const
	{
		++pos;
		size_t d = data >> pos;
		if (d == 0)
			return size();
		return bit_scan_forward(d) + pos;
	}

	FastBitset& operator|=(const FastBitset& r)
	{
		data |= r.data;
		return *this;
	}
};

}
#endif

#endif /* FASTBITSET_H_ */
