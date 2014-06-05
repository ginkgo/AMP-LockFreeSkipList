/*
 * Strategy2LazySsspLocalDistances.h
 *
 *  Created on: Oct 9, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGY2LAZYSSSPLOCALDISTANCES_H_
#define STRATEGY2LAZYSSSPLOCALDISTANCES_H_

#include <unordered_map>
#include <limits>

namespace pheet {

struct Strategy2LazySsspLocalDistancesValue {
	Strategy2LazySsspLocalDistancesValue()
	:value(std::numeric_limits<size_t>::max()){
	}

	size_t value;
};
template <class Pheet>
class Strategy2LazySsspLocalDistances {
public:
	Strategy2LazySsspLocalDistances() {}
	~Strategy2LazySsspLocalDistances() {}

	size_t& operator[](size_t index) {
		return distances[index].value;
	}
private:
	std::unordered_map<size_t, Strategy2LazySsspLocalDistancesValue> distances;
};

} /* namespace pheet */
#endif /* STRATEGY2LAZYSSSPLOCALDISTANCES_H_ */
