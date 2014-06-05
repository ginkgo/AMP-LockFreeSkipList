/*
 * ScalarMonoid.h
 *
 *  Created on: 02.09.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SCALARMONOID_H_
#define SCALARMONOID_H_

/*
 *
 */
namespace pheet {

template <typename T, template <typename S> class Operation>
class ScalarMonoid {
public:
	typedef T const& OutputType;
	typedef ScalarMonoid<T, Operation> Monoid;

	ScalarMonoid();
	ScalarMonoid(Monoid const& other);
	~ScalarMonoid();

	void left_reduce(Monoid& other);
	void right_reduce(Monoid& other);
	void reset();

	void put(T value);
	OutputType get();

private:
	T value;
	Operation<T> reduce_op;
};

template <typename T, template <typename S> class Operation>
ScalarMonoid<T, Operation>::ScalarMonoid() {
	value = reduce_op.get_identity();
}

template <typename T, template <typename S> class Operation>
ScalarMonoid<T, Operation>::ScalarMonoid(Monoid const&) {
	value = reduce_op.get_identity();
}

template <typename T, template <typename S> class Operation>
ScalarMonoid<T, Operation>::~ScalarMonoid() {

}

template <typename T, template <typename S> class Operation>
void ScalarMonoid<T, Operation>::left_reduce(Monoid& other) {
	value = reduce_op(other.value, value);
}

template <typename T, template <typename S> class Operation>
void ScalarMonoid<T, Operation>::right_reduce(Monoid& other) {
	value = reduce_op(value, other.value);
}

template <typename T, template <typename S> class Operation>
void ScalarMonoid<T, Operation>::reset() {
	value = reduce_op.get_identity();
}

template <typename T, template <typename S> class Operation>
void ScalarMonoid<T, Operation>::put(T value) {
	this->value = reduce_op(this->value, value);
}

template <typename T, template <typename S> class Operation>
typename ScalarMonoid<T, Operation>::OutputType ScalarMonoid<T, Operation>::get() {
	return value;
}

}

#endif /* SCALARMONOID_H_ */
