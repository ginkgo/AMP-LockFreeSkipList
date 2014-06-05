/*
 * Strategy2UTSStrategy.h
 *
 *  Created on: 16.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGY2UTSSTRATEGY_H_
#define STRATEGY2UTSSTRATEGY_H_

#include <pheet/pheet.h>

namespace pheet {

template <class Pheet>
class Strategy2UTSStrategy : public Pheet::Environment::BaseStrategy {
public:
    typedef Strategy2UTSStrategy<Pheet> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

    typedef typename Pheet::Environment::TaskStorage TaskStorage;
    typedef typename TaskStorage::Place TaskStoragePlace;

    typedef typename Pheet::Place Place;

	Strategy2UTSStrategy() {}
	Strategy2UTSStrategy(int height)
	:height(height){}

	Strategy2UTSStrategy(Self& other)
	: BaseStrategy(other),
	  height(other.height) {}

	Strategy2UTSStrategy(Self&& other)
	: BaseStrategy(other),
	  height(other.height) {}

	~Strategy2UTSStrategy() {}

	Self& operator=(Self&& other) {
		height = other.height;
		return *this;
	}

	/*
	 * Checks whether spawn can be converted to a function call
	 */
	inline bool can_call(TaskStoragePlace* p) {
		if(height >= 8)
			return p->size() >= 4;
		else
			return p->size() >= 12 - (unsigned)height;
	}

	static void print_name() {
		std::cout << "Strategy2";
	}

private:
	int height;
};

} /* namespace pheet */
#endif /* STRATEGY2UTSSTRATEGY_H_ */
