/*
 * ListMonoid.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LISTMONOID_H_
#define LISTMONOID_H_

namespace pheet {

template <typename T, template <typename> class ListT>
class ListMonoid {
public:
	typedef ListT<T> List;
	typedef List const& OutputType;
	typedef ListMonoid<T, ListT> Monoid;

	ListMonoid() {}
	ListMonoid(Monoid const& other) {} // No copying here!!!
	~ListMonoid() {}

	void left_reduce(Monoid& other) {
		list.splice(list.begin(), other.list);
		pheet_assert(other.list.empty());
	}
	void right_reduce(Monoid& other) {
		list.splice(list.end(), other.list);
		pheet_assert(other.list.empty());
	}
	void reset() {
		list.clear();
	}

	void put(T const& value) {
		list.push_back(value);
	}
	OutputType get() {
		return list;
	}

private:
	List list;
};

} /* namespace pheet */
#endif /* LISTMONOID_H_ */
