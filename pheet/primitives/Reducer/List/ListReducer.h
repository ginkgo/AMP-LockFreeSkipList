/*
 * ListReducer.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LISTREDUCER_H_
#define LISTREDUCER_H_

#include <list>
#include "../Ordered/OrderedReducer.h"
#include "ListMonoid.h"

/*
 *
 */
namespace pheet {

template <class Pheet, typename T, template <typename> class ListT, template <typename, template <typename> class> class MonoidT>
class ListReducerImpl {
public:
	typedef ListReducerImpl<Pheet, T, ListT, MonoidT> Self;
	typedef ListT<T> List;
	typedef MonoidT<T, ListT> Monoid;

	ListReducerImpl();
	ListReducerImpl(Self& other);
	ListReducerImpl(Self&& other);
	~ListReducerImpl();

	void add(T const& value);

	List const& get_list();
private:
	typedef OrderedReducer<Pheet, Monoid> Reducer;
	Reducer reducer;
};

template <class Pheet, typename T, template <typename> class ListT, template <typename, template <typename> class> class MonoidT>
ListReducerImpl<Pheet, T, ListT, MonoidT>::ListReducerImpl() {

}

template <class Pheet, typename T, template <typename> class ListT, template <typename, template <typename> class> class MonoidT>
ListReducerImpl<Pheet, T, ListT, MonoidT>::ListReducerImpl(Self& other)
: reducer(other.reducer) {

}

template <class Pheet, typename T, template <typename> class ListT, template <typename, template <typename> class> class MonoidT>
ListReducerImpl<Pheet, T, ListT, MonoidT>::ListReducerImpl(Self&& other)
: reducer(std::move(other.reducer)) {

}

template <class Pheet, typename T, template <typename> class ListT, template <typename, template <typename> class> class MonoidT>
ListReducerImpl<Pheet, T, ListT, MonoidT>::~ListReducerImpl() {

}

template <class Pheet, typename T, template <typename> class ListT, template <typename, template <typename> class> class MonoidT>
void ListReducerImpl<Pheet, T, ListT, MonoidT>::add(T const& value) {
	reducer.add_data(value);
}


template <class Pheet, typename T, template <typename> class ListT, template <typename, template <typename> class> class MonoidT>
ListT<T> const& ListReducerImpl<Pheet, T, ListT, MonoidT>::get_list() {
	return reducer.get_data();
}

template <typename T>
using ListReducerDefaultList = std::list<T>;

template <class Pheet, typename T>
using ListReducer = ListReducerImpl<Pheet, T, ListReducerDefaultList, ListMonoid>;

}

#endif /* LISTREDUCER_H_ */
