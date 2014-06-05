/*
 * align.h
 *
 *  Created on: 07.03.2014
 *      Author: Manuel Pöter, Martin Wimmer
 *     License: Boost Software License 1.0
 */

#ifndef ALIGN_H_
#define ALIGN_H_

#include <memory>

namespace pheet {

template <typename T, unsigned alignment>
class aligned_data
{
	void* raw;
	void* aligned;
public:
	aligned_data(): raw(nullptr), aligned(nullptr) {}

	aligned_data(size_t elements)
	{
		size_t size = sizeof(T) * elements;
		size_t totalSize = size + alignment - 1;
		raw = malloc(totalSize);
		aligned = raw;
		// Since std::align is not yet supported in GCC we do this manually. Assumes alignment is power of two
		aligned = reinterpret_cast<void*>(reinterpret_cast<size_t>(static_cast<char *>(raw)+alignment - 1) & ~ (static_cast<size_t>(alignment) - 1));

/*		aligned = std::align(alignment, size, aligned, totalSize);
		if (aligned == 0)
			throw std::runtime_error("Alignment of allocation failed");*/
	}

	aligned_data(aligned_data && other) :
		raw(std::move(other.raw)),
		aligned(std::move(other.aligned))
	{
		other.raw = nullptr;
		other.aligned = nullptr;
	}

	aligned_data& operator=(aligned_data&& other) {
		raw = std::move(other.raw);
		aligned = std::move(other.aligned);
		other.raw = nullptr;
		other.aligned = nullptr;
		return *this;
	}

	~aligned_data() { if(raw != nullptr) free(raw); }

	T* ptr() { return static_cast<T*>(aligned); }
};

}


#endif /* ALIGN_H_ */
