/*
 * PrefixSumInitTask.h
 *
 *  Created on: 07.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0
 */

#ifndef PREFIXSUMINITTASK_H_
#define PREFIXSUMINITTASK_H_

namespace pheet {

template <class Pheet>
class PrefixSumInitTask : public Pheet::Task {
public:
	typedef PrefixSumInitTask<Pheet> Self;

	PrefixSumInitTask(unsigned int* data, size_t size, int type)
	:data(data), size(size), type(type) {}
	virtual ~PrefixSumInitTask() {}

	virtual void operator()() {
		// Assumption: Cache line size = 1024 or smaller
		if(size >= 2048) {
			size_t middle = size >> 1;
			// Align to cache lines
			middle = (middle + 1023) & ~ 1023;

			Pheet::template spawn<Self>(data + middle, size - middle, type);
			Pheet::template call<Self>(data, middle, type);
		}
		else {
			switch(type) {
			case 0:
				// 1s
				{
					for(size_t i = 0; i < size; ++i) {
						data[i] = 1;
					}
				}
				break;
			default:
				throw "unknown type for prefix sum";
			}
		}
	}

private:
	unsigned int* data;
	size_t size;
	int type;
};

} /* namespace pheet */
#endif /* PREFIXSUMINITTASK_H_ */
