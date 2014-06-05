/*
 * MaxReducer.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef MAXREDUCER_H_
#define MAXREDUCER_H_

#include "../Ordered/OrderedReducer.h"
#include "../Ordered/ScalarMonoid.h"

#include <limits>

/*
 *
 */
namespace pheet {

template <typename T>
struct MaxOperation {
	T operator()(T x, T y);
	T get_identity();
};

template <typename T>
T MaxOperation<T>::operator()(T x, T y) {
	return std::max(x, y);
}

template <typename T>
T MaxOperation<T>::get_identity() {
	return std::numeric_limits<T>::min();
}

template <class Pheet, typename T, template <typename S> class M = MaxOperation >
class MaxReducer {
public:
	MaxReducer();
	MaxReducer(MaxReducer<Pheet, T, M>& other);
	MaxReducer(MaxReducer<Pheet, T, M>&& other);
	~MaxReducer();

	void add_value(T const& value);
	void add(T const& value) {
		add_value(value);
	}
	T const& get_max();
private:
	typedef OrderedReducer<Pheet, ScalarMonoid<T, M> > Reducer;
	Reducer reducer;
};

template <class Pheet, typename T, template <typename S> class M>
MaxReducer<Pheet, T, M>::MaxReducer() : reducer() {

}

template <class Pheet, typename T, template <typename S> class M>
MaxReducer<Pheet, T, M>::MaxReducer(MaxReducer<Pheet, T, M>& other)
: reducer(other.reducer) {

}

template <class Pheet, typename T, template <typename S> class M>
MaxReducer<Pheet, T, M>::MaxReducer(MaxReducer<Pheet, T, M>&& other)
: reducer(std::move(other.reducer)) {

}

template <class Pheet, typename T, template <typename S> class M>
MaxReducer<Pheet, T, M>::~MaxReducer() {

}

template <class Pheet, typename T, template <typename S> class M>
void MaxReducer<Pheet, T, M>::add_value(T const& value) {
	reducer.add_data(value);
}

template <class Pheet, typename T, template <typename S> class M>
T const& MaxReducer<Pheet, T, M>::get_max() {
	return reducer.get_data();
}

}

#endif /* MAXREDUCER_H_ */
