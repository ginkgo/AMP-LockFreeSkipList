/*
 * RecursiveParallelPrefixSumVectorized2InstructionSet.h
 *
 *  Created on: Mar 23, 2014
 *      Author: Manuel P�ter
 *	   License: Boost Software License 1.0
 */

#ifndef RECURSIVEPARALLELPREFIXSUMVECTORIZED2INSTRUCTIONSET_H_
#define RECURSIVEPARALLELPREFIXSUMVECTORIZED2INSTRUCTIONSET_H_

namespace pheet {
	enum class VectorizationInstructionSet {
#ifdef __MIC__
		MIC,
#else
		SSE2,
		SSE4,
		AVX2
#endif
	};

	template <VectorizationInstructionSet InstructionSet>
	class VectorizationInstructionSetTag {};

	template <VectorizationInstructionSet InstructionSet>
	struct OperationsDef;

#ifdef __MIC__
	template <>
	struct OperationsDef<VectorizationInstructionSet::MIC>
	{
		using Register = __m512i;

		NOT YET IMPLEMENTED!
		/*static Register zero() { return _mm512_setzero_epi32(); }
		static Register load(unsigned int* p) { return _mm512_load_epi32((Register*)p); }
		static Register add(Register a, Register b) { return _mm512_add_epi32(a, b); }
		static unsigned int horizontal_sum(Register v);
		static Register broadcast(unsigned int v) { return _mm512_set1_epi32(v); }
		static Register prefixSum(Register& v)
		{
			const size_t elemsPerVector = sizeof(__m512i) / sizeof(unsigned int);
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
		static void store(unsigned int* p, Register v);
		static Register broadcastHighestElement(Register v);*/
	};
#else
	template <>
	struct OperationsDef<VectorizationInstructionSet::SSE2>
	{
		using Register = __m128i;

		static Register zero() { return _mm_setzero_si128(); }
		static Register load(unsigned int* p) { return _mm_load_si128((__m128i*)p); }
		static Register add(Register a, Register b) { return _mm_add_epi32(a, b); }
		static unsigned int horizontal_sum(Register v)
		{
			union
			{
				Register reg;
				unsigned int elems[4];
			} data;
			_mm_store_si128(&data.reg, v);
			
			return data.elems[0] + data.elems[1] + data.elems[2] + data.elems[3];
		}
		static Register broadcast(unsigned int v) { return _mm_set1_epi32(v); }
		static Register prefixSum(Register& v)
		{
			__m128i t = _mm_slli_si128(v, sizeof(unsigned int));
			v = _mm_add_epi32(v, t);
			t = _mm_slli_si128(v, 2 * sizeof(unsigned int));
			v = _mm_add_epi32(v, t);
			return v;
		}
		static Register shiftInFirstElementFromRight(Register& v, Register& other)
		{
			auto t = _mm_srli_si128(v, sizeof(unsigned int));
			v = _mm_unpacklo_epi32(other, v);
			return _mm_unpacklo_epi64(v, t);
		}
		static void store(unsigned int* p, Register v) { _mm_store_si128((Register*)p, v); }
		static Register broadcastHighestElement(Register v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3)); }
		static unsigned int getFirstElement(Register& v)
		{
			return _mm_cvtsi128_si32(v);
		}
	};
 
#ifdef __SSE4__
	template <>
	struct OperationsDef<VectorizationInstructionSet::SSE4>
	{
		using Register = __m128i;

		static Register zero() { return _mm_setzero_si128(); }
		static Register load(unsigned int* p) { return _mm_load_si128((__m128i*)p); }
		static Register add(Register a, Register b) { return _mm_add_epi32(a, b); }
		static unsigned int horizontal_sum(Register v)
		{
			v = _mm_hadd_epi32(v, v);
			v = _mm_hadd_epi32(v, v);
			return _mm_cvtsi128_si32(v);
		}
		static Register broadcast(unsigned int v) { return _mm_set1_epi32(v); }
		static Register prefixSum(Register& v)
		{
			__m128i t = _mm_slli_si128(v, 4); //sizeof(unsigned int));
			v = _mm_add_epi32(v, t);
			t = _mm_slli_si128(v, 2 * 4); //sizeof(unsigned int));
			v = _mm_add_epi32(v, t);
			return v;
		}
		static Register shiftInFirstElementFromRight(Register& v, Register& other)
		{
			v = _mm_slli_si128(v, 4); //sizeof(unsigned int));
			return _mm_blend_epi16(v, other, 3);
		}
		static void store(unsigned int* p, Register v) { _mm_store_si128((Register*)p, v); }
		static Register broadcastHighestElement(Register v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3)); }
		static unsigned int getFirstElement(Register& v)
		{
			return _mm_cvtsi128_si32(v);
		}
	};
#endif

#ifdef __AVX2__
	template <>
	struct OperationsDef<VectorizationInstructionSet::AVX2>
	{
		using Register = __m256i;

		static Register zero() { return _mm256_setzero_si256(); }
		static Register load(unsigned int* p) { return *((__m256i*)p); }
		static Register add(Register a, Register b) { return _mm256_add_epi32(a, b); }
		static unsigned int horizontal_sum(Register v)
		{
			auto upper = _mm256_permute4x64_epi64(v, 2 | (3 << 2));
			v = _mm256_hadd_epi32(v, upper);
			v = _mm256_hadd_epi32(v, v);
			v = _mm256_hadd_epi32(v, v);
			return _mm_cvtsi128_si32(_mm256_extracti128_si256(v, 0));
		}
		static Register broadcast(unsigned int v) { return _mm256_set1_epi32(v); }
		static Register prefixSum(Register& v)
		{
			auto t = _mm256_bslli_epi128(v, sizeof(unsigned int));
			v = _mm256_add_epi32(v, t);
			t = _mm256_bslli_epi128(v, 2 * sizeof(unsigned int));
			v = _mm256_add_epi32(v, t);
			auto v2 = _mm256_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3));
			v2 = _mm256_permute2x128_si256(zero(), v2, 2 << 4);
			v = _mm256_add_epi32(v, v2);
			return v;

		}
		static Register shiftInFirstElementFromRight(Register& v, Register& other)
		{
			auto t = _mm256_bslli_epi128(v, sizeof(unsigned int));
			auto v2 = _mm256_permute2x128_si256(v, v, 0);
			v2 = _mm256_shuffle_epi32(v2, _MM_SHUFFLE(3, 3, 3, 3));
			v = _mm256_blend_epi32(t, v2, 1 << 4);
			return _mm256_blend_epi32(v, other, 1);
		}
		static void store(unsigned int* p, Register v) { _mm256_store_si256((Register*)p, v); }
		static Register broadcastHighestElement(Register v)
		{
			v = _mm256_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3));
			return _mm256_permute2x128_si256(v, v, 3 | (3 << 4));
		}
		static unsigned int getFirstElement(Register& v)
		{
			return _mm_cvtsi128_si32(_mm256_extracti128_si256(v, 0));
		}
	};
#endif
#endif

	template <VectorizationInstructionSet InstructionSet> 
	struct PrefixSumCalculator
	{
		using Operations = OperationsDef<InstructionSet>;
		using Register = typename Operations::Register;

		static unsigned int calculateSum(unsigned int* data, size_t length)
		{
			const size_t elemsPerVector = sizeof(Register) / sizeof(data[0]);
			if (length < elemsPerVector)
			{
				unsigned int sum = 0;
				for(size_t i = 0; i < length; ++i)
					sum += data[i];
				return sum;
			}

			const size_t remainingLength = length % elemsPerVector;
			length -= remainingLength; // make length a multiple of elemsPerVector

			pheet_assert(sizeof(data[0]) == 4); // this implementation supports only 32-bit integer elements
			pheet_assert(length % elemsPerVector == 0);
			pheet_assert((size_t)data % sizeof(Register) == 0); // pointer must be aligned to vector size

			auto sum = Operations::zero();
			auto p = data;
			for (auto end = p + length; p != end; p += elemsPerVector)
			{
				auto v = Operations::load(p);
				sum = Operations::add(v, sum);
			}

			unsigned int totalSum = Operations::horizontal_sum(sum);
		
			// post alignment loop
			for (size_t i = 0; i < remainingLength; ++i)
				totalSum += p[i - 1];

			return totalSum;
		}

		static void calculateInclusivePrefixSum(unsigned int* data, size_t length, unsigned int sum)
		{
			const size_t elemsPerVector = sizeof(Register) / sizeof(data[0]);
			if (length < elemsPerVector)
			{
				for (size_t i = 0; i < length; ++i)
				{
					sum += data[i];
					data[i] = sum;
				}
				return;
			}

			const size_t remainingLength = length % elemsPerVector;
			length -= remainingLength; // make length a multiple of elemsPerVector
		
			pheet_assert(length % elemsPerVector == 0);
			pheet_assert((size_t)data % sizeof(Register) == 0); // pointer must be aligned to vector size

			auto prev = Operations::broadcast(sum);
			auto p = data;
			for (auto end = p + length; p != end; p += elemsPerVector)
			{
				auto v = Operations::load(p);


				v = Operations::prefixSum(v);
				v = Operations::add(v, prev);

				Operations::store(p, v);
				prev = Operations::broadcastHighestElement(v);
			}

			// post alignment loop
			
			for (size_t i = 0; i < remainingLength; ++i)
				p[i] += p[i - 1];
		}

		static void calculateExclusivePrefixSum(unsigned int* data, size_t length, unsigned int sum)
		{
			const size_t elemsPerVector = sizeof(Register) / sizeof(data[0]);
			if (length < elemsPerVector)
			{
				for (size_t i = 0; i < length; ++i)
				{
					const auto tmp = data[i];
					data[i] = sum;
					sum += tmp;
				}
				return;
			}

			const size_t remainingLength = length % elemsPerVector;
			length -= remainingLength; // make length a multiple of elemsPerVector
		
			pheet_assert(length % elemsPerVector == 0);
			pheet_assert((size_t)data % sizeof(Register) == 0); // pointer must be aligned to vector size

			auto prev = Operations::broadcast(sum);
			auto p = data;
			for (auto end = p + length; p != end; p += elemsPerVector)
			{
				auto v = Operations::load(p);

				v = Operations::prefixSum(v);
				v = Operations::add(v, prev);
				auto nextPrev = Operations::broadcastHighestElement(v);

				v = Operations::shiftInFirstElementFromRight(v, prev);

				prev = nextPrev;
				Operations::store(p, v);
			}

			// post alignment loop
			sum = Operations::getFirstElement(prev);
			for (size_t i = 0; i < remainingLength; ++i)
			{
				const auto tmp = p[i];
				p[i] = sum;
				sum += tmp;
			}
		}
	};

} /* namespace pheet */
#endif /* RECURSIVEPARALLELPREFIXSUMVECTORIZED2INSTRUCTIONSET_H_ */
