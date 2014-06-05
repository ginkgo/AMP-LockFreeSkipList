/*
 * SumReducer.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SUMREDUCER_H_
#define SUMREDUCER_H_

#include "../Ordered/OrderedReducer.h"
#include "../Ordered/ScalarMonoid.h"
#include "SumOperation.h"

#include <iostream>

/*
 *
 */
namespace pheet {

template <class Pheet, typename T, template <typename S> class Op = SumOperation>
class SumReducer {
public:
	SumReducer();
	SumReducer(SumReducer<Pheet, T, Op>& other);
	SumReducer(SumReducer<Pheet, T, Op>&& other);
	~SumReducer();

	void incr();
	void decr();
	void add(T const& value);
	void sub(T const& value);

	T const& get_sum();

	static void print_name() {
		std::cout << "SumReducer";
	}
private:
	typedef OrderedReducer<Pheet, ScalarMonoid<T, Op> > Reducer;
	Reducer reducer;
};

template <class Pheet, typename T, template <typename S> class Op>
SumReducer<Pheet, T, Op>::SumReducer(): reducer() {

}

template <class Pheet, typename T, template <typename S> class Op>
SumReducer<Pheet, T, Op>::SumReducer(SumReducer<Pheet, T, Op>& other)
: reducer(other.reducer) {

}

template <class Pheet, typename T, template <typename S> class Op>
SumReducer<Pheet, T, Op>::SumReducer(SumReducer<Pheet, T, Op>&& other)
: reducer(std::move(other.reducer)) {

}

template <class Pheet, typename T, template <typename S> class Op>
SumReducer<Pheet, T, Op>::~SumReducer() {

}

template <class Pheet, typename T, template <typename S> class Op>
void SumReducer<Pheet, T, Op>::add(T const& value) {
	reducer.add_data(value);
}

template <class Pheet, typename T, template <typename S> class Op>
void SumReducer<Pheet, T, Op>::sub(T const& value) {
	reducer.add_data(-value);
}

template <class Pheet, typename T, template <typename S> class Op>
void SumReducer<Pheet, T, Op>::incr() {
	reducer.add_data(1);
}

template <class Pheet, typename T, template <typename S> class Op>
void SumReducer<Pheet, T, Op>::decr() {
	reducer.add_data(-1);
}

template <class Pheet, typename T, template <typename S> class Op>
T const& SumReducer<Pheet, T, Op>::get_sum() {
	return reducer.get_data();
}
}

#endif /* SUMREDUCER_H_ */
