/*
 * SumOperation.h
 *
 *  Created on: 02.09.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SUMOPERATION_H_
#define SUMOPERATION_H_

namespace pheet {

template <typename T>
struct SumOperation {
	T operator()(T x, T y);
	T get_identity();
};

template <typename T>
T SumOperation<T>::operator()(T x, T y) {
	return x + y;
}

template <typename T>
T SumOperation<T>::get_identity() {
	return 0;
}

}

#endif /* SUMOPERATION_H_ */
