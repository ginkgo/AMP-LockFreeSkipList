/*
 * RecursiveParallelVectorizedPrefixSumOffsetTask.h
 *
 *  Created on: Nov 06, 2012
 *      Author: Martin Wimmer, Manuel Pöter
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELVECTORIZEDPREFIXSUMOFFSETTASK_H_
#define RECURSIVEPARALLELVECTORIZEDPREFIXSUMOFFSETTASK_H_

#include <pheet/pheet.h>
#include <immintrin.h>

//#define COMPILER_VECTORIZED
namespace pheet {

template <class Pheet, size_t BlockSize>
class RecursiveParallelVectorizedPrefixSumOffsetTask : public Pheet::Task {
public:
	typedef RecursiveParallelVectorizedPrefixSumOffsetTask<Pheet, BlockSize> Self;

	RecursiveParallelVectorizedPrefixSumOffsetTask(unsigned int* data, size_t length, size_t step)
	:data(data), length(length), step(step) {}
	virtual ~RecursiveParallelVectorizedPrefixSumOffsetTask() {}

	virtual void operator()()
	{
#ifdef COMPILER_VECTORIZED
		offsetGeneric();
#else 
#ifdef __MIC__
		offsetMIC();
#else
		offsetSSE();
#endif
#endif
	}

#ifdef COMPILER_VECTORIZED
	void offsetGeneric()
	{
		if(length <= BlockSize)
		{
			if(length == BlockSize)
				--length;

			const unsigned l = (unsigned)length;
			const unsigned int v = *(data - step);
			if (step != 1)
			{
//#pragma vector(always)
#pragma ivdep
				for(unsigned i = 0; i < l; ++i)
					data[i*step] += v;
			}
			else
			{
//#pragma vector(aligned)
#pragma ivdep
				for(unsigned i = 0; i < l; ++i)
					data[i] += v;
			}
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;
			size_t half = BlockSize * (num_blocks >> 1);
			Pheet::template
				spawn<Self>(data, half, step);
			Pheet::template
				spawn<Self>(data + half*step, length - half, step);
		}
	}
#else	
#ifdef __MIC__
	void offsetMIC()
	{
		if(length <= BlockSize)
		{
			if(length == BlockSize)
				--length;

			const size_t elemsPerVector = sizeof(__m512i) / sizeof(data[0]);
			const size_t remainingLength = length % elemsPerVector;				
			__m512i v = _mm512_extload_epi32(data - step, _MM_UPCONV_EPI32_NONE, _MM_BROADCAST_1X16, 0);
							
			if (step != 1)
			{
				unsigned int __attribute__ ((aligned (64))) offset_array[16];
				for (int i = 0; i < 16; ++i)
					offset_array[i] = i * step;
				__m512i offsets = _mm512_load_epi32(offset_array);
			
				auto p = data;
				for(size_t i = 0; i < length / elemsPerVector; ++i, p += elemsPerVector * step) {
					__m512i t = _mm512_i32gather_epi32(offsets, p, 4);
					t = _mm512_add_epi32(v, t);
					_mm512_i32scatter_epi32(p, offsets, t, 4);
				}
				
				if (remainingLength != 0)
				{
					__m512i t = _mm512_i32gather_epi32(offsets, p, 4);
					t = _mm512_add_epi32(v, t);
					__mmask16 mask = _mm512_int2mask((1 << remainingLength) - 1);
					_mm512_mask_i32scatter_epi32(p, mask, offsets, t, 4);
				}
				return;
			}
			else
			{
				length -= remainingLength;
				pheet_assert((size_t)data % sizeof(__m512i) == 0); // pointer must be aligned to vector size
 				
				auto p = data;
				for(auto end = p + length; p != end; p += elemsPerVector)
				{
					__m512i t = _mm512_load_epi32(p);
					t = _mm512_add_epi32(v, t);
					_mm512_store_epi32(p, t);
				}
				
				if (remainingLength != 0)
				{
					__m512i t = _mm512_load_epi32(p);
					t = _mm512_add_epi32(v, t);
					__mmask16 mask = _mm512_int2mask((1 << remainingLength) - 1);
					_mm512_mask_store_epi32(p, mask, t);
				}
			}
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;
			size_t half = BlockSize * (num_blocks >> 1);
			Pheet::template spawn<Self>(data, half, step);
			Pheet::template spawn<Self>(data + half*step, length - half, step);
		}
	}
#else

	void offsetSSE()
	{
		if(length <= BlockSize)
		{
			if(length == BlockSize)
				--length;

			if (step != 1)
			{
				unsigned int v = *(data - step);
				for(size_t i = 0; i < length; ++i)
					data[i*step] += v;
				return;
			}
			else
			{
				const size_t elemsPerVector = sizeof(__m128i) / sizeof(data[0]);
				const size_t remainingLength = length % elemsPerVector;
				unsigned int v1 = *(data - step);
				__m128i v = _mm_set1_epi32((int)v1);
				
				length -= remainingLength;
				pheet_assert((size_t)data % sizeof(__m128i) == 0); // pointer must be aligned to vector size

				auto p = data;
				for(auto end = p + length; p != end; p += elemsPerVector)
				{
					__m128i t = _mm_load_si128((__m128i*)p);
					t = _mm_add_epi32(v, t);
					_mm_store_si128((__m128i*)p, t);
				}

				for(size_t i = 0; i < remainingLength; ++i)
					p[i] += v1;
			}
		}
		else
		{
			size_t num_blocks = ((length - 1) / BlockSize) + 1;
			size_t half = BlockSize * (num_blocks >> 1);
			Pheet::template spawn<Self>(data, half, step);
			Pheet::template spawn<Self>(data + half*step, length - half, step);
		}
	}
#endif
#endif

private:
	unsigned int* data;
	size_t length;
	size_t step;
};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELVECTORIZEDPREFIXSUMOFFSETTASK_H_ */
