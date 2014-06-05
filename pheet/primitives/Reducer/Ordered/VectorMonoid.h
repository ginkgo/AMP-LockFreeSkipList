/*
 * VectorMonoid.h
 *
 *  Created on: 02.09.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef VECTORMONOID_H_
#define VECTORMONOID_H_

/*
 *
 */
namespace pheet {

template <typename T, template <typename S> class Operation>
class VectorMonoid {
public:
	typedef T const* OutputType;
	typedef VectorMonoid<T, Operation> Monoid;

	VectorMonoid(size_t length);
	VectorMonoid(Monoid const& other);
	~VectorMonoid();

	void left_reduce(Monoid& other);
	void right_reduce(Monoid& other);
	void reset();

	void put(size_t i, T value);
	OutputType get();

private:
	Operation<T> reduce_op;
	size_t length;
	T* values;
};

template <typename T, template <typename S> class Operation>
VectorMonoid<T, Operation>::VectorMonoid(size_t length)
: length(length), values(NULL) {

}

template <typename T, template <typename S> class Operation>
VectorMonoid<T, Operation>::VectorMonoid(Monoid const& other)
: length(other.length), values(NULL) {

}

template <typename T, template <typename S> class Operation>
VectorMonoid<T, Operation>::~VectorMonoid() {
	if(values != NULL) {
		delete[] values;
	}
}

template <typename T, template <typename S> class Operation>
void VectorMonoid<T, Operation>::left_reduce(Monoid& other) {
	if(other.values != NULL) {
		if(values == NULL) {
			values = other.values;
			other.values = NULL;
		}
		else {
			for(size_t i = 0; i < length; ++i) {
				values[i] = reduce_op(other.values[i], values[i]);
			}
		}
	}
}

template <typename T, template <typename S> class Operation>
void VectorMonoid<T, Operation>::right_reduce(Monoid& other) {
	if(other.values != NULL) {
		if(values == NULL) {
			values = other.values;
			other.values = NULL;
		}
		else {
			for(size_t i = 0; i < length; ++i) {
				values[i] = reduce_op(values[i], other.values[i]);
			}
		}
	}
}

template <typename T, template <typename S> class Operation>
void VectorMonoid<T, Operation>::reset() {
	if(values != NULL) {
		for(size_t i = 0; i < length; ++i) {
			values[i] = reduce_op.get_identity();
		}
	}
}

template <typename T, template <typename S> class Operation>
void VectorMonoid<T, Operation>::put(size_t i, T value) {
	if(values == NULL) {
		values = new T[length];
		reset();
	}
	this->values[i] = reduce_op(this->values[i], value);
}

template <typename T, template <typename S> class Operation>
typename VectorMonoid<T, Operation>::OutputType VectorMonoid<T, Operation>::get() {
	if(values == NULL) {
		values = new T[length];
		reset();
	}
	return values;
}

}

#endif /* VECTORMONOID_H_ */
