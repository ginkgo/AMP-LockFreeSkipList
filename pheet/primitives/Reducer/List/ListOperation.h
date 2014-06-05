/*
 * SumOperation.h
 *
 *  Created on: 02.09.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LISTOPERATION_H_
#define LISTOPERATION_H_

namespace pheet {

template <typename T>
struct ListOperation {
	T operator()(T x, T y);
	T get_identity();
};

template <typename T>
T ListOperation<T>::operator()(T x, T y) {
	x.insert(x.end(), y.begin(), y.end());
	return x;
}

template <typename T>
T ListOperation<T>::get_identity() {
	return T();
}

}

#endif /* LISTOPERATION_H_ */
