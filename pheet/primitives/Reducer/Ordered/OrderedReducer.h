/*
 * OrderedReducer.h
 *
 *  Created on: 09.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ORDEREDREDUCER_H_
#define ORDEREDREDUCER_H_

#include "../../../settings.h"
#include "OrderedReducerView.h"
#include "../../Backoff/Exponential/ExponentialBackoff.h"

/*
 *
 */
namespace pheet {

template <class Pheet, class Monoid>
class OrderedReducer {
public:
	template <typename ... ConsParams>
	OrderedReducer(ConsParams&& ... params);
	OrderedReducer(OrderedReducer& other);
	OrderedReducer(OrderedReducer&& other);
	~OrderedReducer();

	template <typename ... PutParams>
	void add_data(PutParams&& ... params);
	typename Monoid::OutputType get_data();
private:
	void finalize();
	void minimize();

	typedef OrderedReducerView<Monoid> View;
	typedef typename Pheet::Backoff Backoff;
	View* my_view;
	View* parent_view;
	procs_t place_id;
};

template <class Pheet, class Monoid>
template <typename ... ConsParams>
OrderedReducer<Pheet, Monoid>::OrderedReducer(ConsParams&& ... params)
:my_view(new View(std::forward<ConsParams&&>(params) ...)), parent_view(NULL), place_id(Pheet::get_place_id())
{

}

template <class Pheet, class Monoid>
OrderedReducer<Pheet, Monoid>::OrderedReducer(OrderedReducer& other)
: place_id(Pheet::get_place_id()){
	// Before we create a new view, we should minimize. Maybe this provides us with a view to reuse
	other.minimize();

	my_view = other.my_view;
	parent_view = my_view->create_parent_view();
	other.my_view = parent_view;
}

template <class Pheet, class Monoid>
OrderedReducer<Pheet, Monoid>::OrderedReducer(OrderedReducer&& other)
: place_id(Pheet::get_place_id()){
	// Move semantics. The other view gets invalidated!
	my_view = other.my_view;
	other.my_view = nullptr;
}

template <class Pheet, class Monoid>
OrderedReducer<Pheet, Monoid>::~OrderedReducer() {
	finalize();
}

template <class Pheet, class Monoid>
void OrderedReducer<Pheet, Monoid>::finalize() {
	if(my_view != NULL) {
		minimize();
		if(parent_view == NULL) {
			pheet_assert(my_view->is_reduced());
			delete my_view;
		}
		else if(place_id == Pheet::get_place_id()) {
			// This reducer has only been used locally. No need for sync
			parent_view->set_local_predecessor(my_view);
		}
		else {
			// Notify parent view
			parent_view->set_predecessor(my_view);
		}
	}
}

/*
 * Do folding and reduce if possible. May free some memory that we can reuse
 */
template <class Pheet, class Monoid>
void OrderedReducer<Pheet, Monoid>::minimize() {
	pheet_assert(my_view != NULL);
	if(place_id == Pheet::get_place_id()) {
		my_view = my_view->fold();
		my_view->reduce(true);
	}
	else {
		// This reducer has been moved to another thread
		my_view->reduce(false);
	}
}

template <class Pheet, class Monoid>
template <typename ... PutParams>
void OrderedReducer<Pheet, Monoid>::add_data(PutParams&& ... params) {
	pheet_assert(my_view != NULL);
	if(place_id == Pheet::get_place_id()) {
		my_view = my_view->fold();
	}
	my_view->add_data(std::forward<PutParams&&>(params) ...);
}

template <class Pheet, class Monoid>
typename Monoid::OutputType OrderedReducer<Pheet, Monoid>::get_data() {
	pheet_assert(my_view != NULL);
	Backoff bo;
	if(!my_view->is_reduced()) {
		while(true) {
			minimize();
			if(my_view->is_reduced()) {
				break;
			}
			bo.backoff();
		}
	}
	return my_view->get_data();
}

}

#endif /* ORDEREDREDUCER_H_ */
