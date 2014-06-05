/*
 * VectorSumReducer.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef VECTORSUMREDUCER_H_
#define VECTORSUMREDUCER_H_

#include "../Ordered/OrderedReducer.h"
#include "../Ordered/VectorMonoid.h"
#include "SumOperation.h"

/*
 *
 */
namespace pheet {

template <class Pheet, typename T, template <typename S> class Op = SumOperation>
class VectorSumReducer {
public:
	VectorSumReducer(size_t length);
	VectorSumReducer(VectorSumReducer<Pheet, T, Op>& other);
	VectorSumReducer(VectorSumReducer<Pheet, T, Op>&& other);
	~VectorSumReducer();

	void incr(size_t i);
	void decr(size_t i);
	void add(size_t i, T value);
	void sub(size_t i, T value);

	T const* get_sum();
private:
	typedef OrderedReducer<Pheet, VectorMonoid<T, Op> > Reducer;
	Reducer reducer;
};

template <class Pheet, typename T, template <typename S> class Op>
VectorSumReducer<Pheet, T, Op>::VectorSumReducer(size_t length)
: reducer(length) {

}

template <class Pheet, typename T, template <typename S> class Op>
VectorSumReducer<Pheet, T, Op>::VectorSumReducer(VectorSumReducer<Pheet, T, Op>& other)
: reducer(other.reducer) {

}

template <class Pheet, typename T, template <typename S> class Op>
VectorSumReducer<Pheet, T, Op>::VectorSumReducer(VectorSumReducer<Pheet, T, Op>&& other)
: reducer(std::move(other.reducer)) {

}

template <class Pheet, typename T, template <typename S> class Op>
VectorSumReducer<Pheet, T, Op>::~VectorSumReducer() {

}

template <class Pheet, typename T, template <typename S> class Op>
void VectorSumReducer<Pheet, T, Op>::add(size_t i, T value) {
	reducer.add_data(i, value);
}

template <class Pheet, typename T, template <typename S> class Op>
void VectorSumReducer<Pheet, T, Op>::sub(size_t i, T value) {
	reducer.add_data(i, -value);
}

template <class Pheet, typename T, template <typename S> class Op>
void VectorSumReducer<Pheet, T, Op>::incr(size_t i) {
	reducer.add_data(i, 1);
}

template <class Pheet, typename T, template <typename S> class Op>
void VectorSumReducer<Pheet, T, Op>::decr(size_t i) {
	reducer.add_data(i, -1);
}

template <class Pheet, typename T, template <typename S> class Op>
T const* VectorSumReducer<Pheet, T, Op>::get_sum() {
	return reducer.get_data();
}

}

#endif /* VECTORSUMREDUCER_H_ */
