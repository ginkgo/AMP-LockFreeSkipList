/*
 * MinReducer.h
 *
 *  Created on: Nov 7, 2011
 *      Author: Martin Wimmer
 */

#ifndef MINREDUCER_H_
#define MINREDUCER_H_

#include "../Max/MaxReducer.h"

namespace pheet {

template <typename T>
struct MinOperation {
	T operator()(T x, T y);
	T get_identity();
};

template <typename T>
T MinOperation<T>::operator()(T x, T y) {
	return std::min(x, y);
}

template <typename T>
T MinOperation<T>::get_identity() {
	return std::numeric_limits<T>::max();
}

template <class Pheet, typename T, template <typename S> class M = MinOperation >
class MinReducer : public MaxReducer<Pheet, T, M> {
public:
	MinReducer();
	MinReducer(MinReducer<Pheet, T, M>& other);
	MinReducer(MinReducer<Pheet, T, M>&& other);
	~MinReducer();

	T const& get_min();
};

template <class Pheet, typename T, template <typename S> class M>
inline MinReducer<Pheet, T, M>::MinReducer() {

}

template <class Pheet, typename T, template <typename S> class M>
inline MinReducer<Pheet, T, M>::MinReducer(MinReducer<Pheet, T, M>& other)
: MaxReducer<Pheet, T, M>(other) {

}

template <class Pheet, typename T, template <typename S> class M>
inline MinReducer<Pheet, T, M>::MinReducer(MinReducer<Pheet, T, M>&& other)
: MaxReducer<Pheet, T, M>(std::move(other)) {

}

template <class Pheet, typename T, template <typename S> class M>
inline MinReducer<Pheet, T, M>::~MinReducer() {

}

template <class Pheet, typename T, template <typename S> class M>
inline T const& MinReducer<Pheet, T, M>::get_min() {
	return MaxReducer<Pheet, T, M>::get_max();
}

}

#endif /* MINREDUCER_H_ */
