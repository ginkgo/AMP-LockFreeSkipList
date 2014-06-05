/*
 * RecursiveParallelVectorizedPrefixSum.h
 *
 *  Created on: Nov 06, 2012
 *      Author: Martin Wimmer, Manuel P�ter
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELVECTORIZEDPREFIXSUM_H_
#define RECURSIVEPARALLELVECTORIZEDPREFIXSUM_H_

#include <iostream>
#include <pheet/pheet.h>
#include "RecursiveParallelVectorizedPrefixSumOffsetTask.h"
#include "RecursiveParallelVectorizedPrefixSumPerformanceCounters.h"

#include <immintrin.h>

#ifdef __MIC__
#include <zmmintrin.h>
#endif

namespace pheet {

template <class Pheet, size_t BlockSize>
class RecursiveParallelVectorizedPrefixSumImpl : public Pheet::Task {
public:
	typedef RecursiveParallelVectorizedPrefixSumImpl<Pheet, BlockSize> Self;
	typedef RecursiveParallelVectorizedPrefixSumOffsetTask<Pheet, BlockSize> OffsetTask;
	typedef RecursiveParallelVectorizedPrefixSumPerformanceCounters<Pheet> PerformanceCounters;

	RecursiveParallelVectorizedPrefixSumImpl(unsigned int* data, size_t length)
	:data(data), length(length), step(1), root(true) {

	}

	RecursiveParallelVectorizedPrefixSumImpl(unsigned int* data, size_t length, size_t step, bool root)
	:data(data), length(length), step(step), root(root) {

	}

	RecursiveParallelVectorizedPrefixSumImpl(unsigned int* data, size_t length, PerformanceCounters& pc)
	:data(data), length(length), step(1), root(true), pc(pc) {

	}

	RecursiveParallelVectorizedPrefixSumImpl(unsigned int* data, size_t length, size_t step, bool root, PerformanceCounters& pc)
	:data(data), length(length), step(step), root(root), pc(pc) {

	}

	virtual ~RecursiveParallelVectorizedPrefixSumImpl() {}

	virtual void operator()() {
		if(length <= BlockSize) {
			calcLocalPrefixSum();
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;
			size_t half = BlockSize * (num_blocks >> 1);
			if(root)
			{
				{
					typename Pheet::Finish f;
					Pheet::template spawn<Self>(data, half, step, false, pc);
					Pheet::template spawn<Self>(data + half*step, length - half, step, false, pc);
				}
//std::cout << data[1024] << std::endl;
				if(num_blocks > 2)
				{
					Pheet::template finish<Self>(data + (BlockSize - 1)*step, num_blocks - 1, BlockSize * step, true, pc); 
				}
//				std::cout << data[1024] << std::endl;

				Pheet::template call<OffsetTask>(data + (BlockSize*step), length - BlockSize, step);
			}
			else {
				Pheet::template spawn<Self>(data, half, step, false, pc);
				Pheet::template spawn<Self>(data + half*step, length - half, step, false, pc);
			}
		}
	}

	void calcLocalPrefixSum()
	{
#ifdef __MIC__
		calcLocalPrefixSumMIC();
#else
		calcLocalPrefixSumSSE();
#endif
	}
	
#ifdef __MIC__
	__m512i calcVectorPrefixSum(__m512i  v);
	void calcLocalPrefixSumMIC();
#else
	__m128i calcVectorPrefixSumSSE(__m128i  v);
	void calcLocalPrefixSumSSE();
#endif

	static char const name[];

private:
	unsigned int* data;
	size_t length;
	size_t step;
	bool root;
	PerformanceCounters pc;
};

#ifdef __MIC__
template <class Pheet, size_t BlockSize>
__m512i RecursiveParallelVectorizedPrefixSumImpl<Pheet, BlockSize>::calcVectorPrefixSum(__m512i  v)
{
	const size_t elemsPerVector = sizeof(__m512i) / sizeof(data[0]);
	__m512i zero = _mm512_setzero_epi32();
	__m512i t = _mm512_alignr_epi32(v, zero, (elemsPerVector - 1) * (16 / elemsPerVector));
	v = _mm512_add_epi32(v, t);

	t = _mm512_alignr_epi32(v, zero, (elemsPerVector - 2) * (16 / elemsPerVector));
	v = _mm512_add_epi32(v, t);

	t = _mm512_alignr_epi32(v, zero, (elemsPerVector - 4) * (16 / elemsPerVector));
	v = _mm512_add_epi32(v, t);

	t = _mm512_alignr_epi32(v, zero, (elemsPerVector - 8) * (16 / elemsPerVector));
	return _mm512_add_epi32(v, t);
}

template <class Pheet, size_t BlockSize>
void RecursiveParallelVectorizedPrefixSumImpl<Pheet, BlockSize>::calcLocalPrefixSumMIC()
{
	const size_t elemsPerVector = sizeof(__m512i) / sizeof(data[0]);
	if (length < elemsPerVector)
	{
		for(size_t i = 1; i < length; ++i)
			data[i*step] += data[(i - 1)*step];
		return;
	}

	__m512i permute = _mm512_set1_epi32(15);
	const size_t remainingLength = length % elemsPerVector;
	length -= remainingLength; // make length a multiple of elemsPerVector
	const size_t iterations = length / elemsPerVector;

	if (step != 1)
	{
		unsigned int __attribute__ ((aligned (64))) offset_array[16];
		for (int i = 0; i < 16; ++i)
			offset_array[i] = i * step;
		__m512i offsets = _mm512_load_epi32(offset_array);
		__m512i prev = _mm512_setzero_epi32();

		auto p = data;
		for(size_t i = 0; i < iterations; ++i, p += elemsPerVector * step) {
			__m512i t = _mm512_i32gather_epi32(offsets, p, 4);
			t = calcVectorPrefixSum(t);
			t = _mm512_add_epi32(t, prev);
			_mm512_i32scatter_epi32(p, offsets, t, 4);
			prev = _mm512_permutevar_epi32(permute, t); // broadcast the highest element to the full vector
		}

		if (remainingLength != 0)
		{
			__mmask16 mask = _mm512_int2mask((1 << remainingLength) - 1);
			__m512i t = _mm512_mask_i32gather_epi32(prev, mask, offsets, p, 4);
			t = calcVectorPrefixSum(t);
			t = _mm512_add_epi32(t, prev);
			_mm512_mask_i32scatter_epi32(p, mask, offsets, t, 4);
		}

		return;
	}

	pheet_assert(step == 1);
	pheet_assert(elemsPerVector == 16); // the current implementation supports only 16 32-bit integer elements
	pheet_assert(length % elemsPerVector == 0);
	pheet_assert((size_t)data % sizeof(__m512i) == 0); // pointer must be aligned to vector size

	__m512i zero = _mm512_setzero_epi32();
	__m512i prev = _mm512_setzero_epi32();
	auto p = data;

	for (auto end = p + length; p != end; p += elemsPerVector)
	{
		__m512i v1 = _mm512_load_epi32(p);

		v1 = calcVectorPrefixSum(v1);
		v1 = _mm512_add_epi32(v1, prev);

		_mm512_store_epi32(p, v1);
		prev = _mm512_permutevar_epi32(permute, v1); // broadcast the highest element to the full vector
	}
	MEMORY_FENCE();

	// post alignment loop
	for (size_t i = 0; i < remainingLength; ++i)
		p[i] += p[i - 1];
}
#else
template <class Pheet, size_t BlockSize>
__m128i RecursiveParallelVectorizedPrefixSumImpl<Pheet, BlockSize>::calcVectorPrefixSumSSE(__m128i  v)
{
//	const size_t elemsPerVector = sizeof(__m128i) / sizeof(data[0]);
	static_assert(/* elemsPerVector */ sizeof(__m128i) / sizeof(data[0]) == 4, "Only 32 bit integers are supported");

	// Need temporary or won't compile under GCC 4.8
	size_t const tmp = sizeof(data[0]);
	__m128i t = _mm_slli_si128(v, tmp);
	v = _mm_add_epi32(v, t);

	// Need temporary or won't compile under GCC 4.8
	size_t const tmp2 = sizeof(data[0])*2;
	t = _mm_slli_si128(v, tmp2);
	return _mm_add_epi32(v, t);
}

template <class Pheet, size_t BlockSize>
void RecursiveParallelVectorizedPrefixSumImpl<Pheet, BlockSize>::calcLocalPrefixSumSSE()
{
	const size_t elemsPerVector = sizeof(__m128i) / sizeof(data[0]);
	if (length < elemsPerVector || step != 1)
	{
		for(size_t i = 1; i < length; ++i)
			data[i*step] += data[(i - 1)*step];
		return;
	}

	const size_t remainingLength = length % elemsPerVector;
	length -= remainingLength; // make length a multiple of elemsPerVector
//	const size_t iterations = length / elemsPerVector;

	pheet_assert(step == 1);
	pheet_assert(elemsPerVector == 4); // this implementation supports only 4 32-bit integer elements
	pheet_assert(length % elemsPerVector == 0);
	pheet_assert((size_t)data % sizeof(__m128i) == 0); // pointer must be aligned to vector size

//	__m128i zero = _mm_setzero_si128();
	__m128i prev = _mm_setzero_si128();
	auto p = data;
	for (auto end = p + length; p != end; p += elemsPerVector)
	{
		__m128i v1 = _mm_load_si128((__m128i*)p);

		v1 = calcVectorPrefixSumSSE(v1);
		v1 = _mm_add_epi32(v1, prev);

		_mm_store_si128((__m128i*)p, v1);
		prev = _mm_shuffle_epi32(v1, _MM_SHUFFLE(3, 3, 3, 3)); // broadcast the highest element to the full vector
	}

	// post alignment loop
	for (size_t i = 0; i < remainingLength; ++i)
		p[i] += p[i - 1];
}
#endif

template <class Pheet, size_t BlockSize>
char const RecursiveParallelVectorizedPrefixSumImpl<Pheet, BlockSize>::name[] = "RecursiveParallelVectorizedPrefixSum";

template <class Pheet>
using RecursiveParallelVectorizedPrefixSum = RecursiveParallelVectorizedPrefixSumImpl<Pheet, 4096>;

} /* namespace pheet */
#endif /* RECURSIVEPARALLELVECTORIZEDPREFIXSUM_H_ */
