/*
 * OrderedReducerView.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ORDEREDREDUCERVIEW_H_
#define ORDEREDREDUCERVIEW_H_

#include "../../../settings.h"
#include "../../../misc/atomics.h"
#include "../../../misc/types.h"

#include <stdint.h>

/*
 *
 */
namespace pheet {

template <class Monoid>
class OrderedReducerView {
public:
	template <typename ... ConsParams>
	OrderedReducerView(ConsParams&& ... params);
	OrderedReducerView(Monoid const& template_monoid);
	~OrderedReducerView();

	OrderedReducerView<Monoid>* fold();
	OrderedReducerView<Monoid>* create_parent_view();
	void reduce(bool allow_local);
	bool is_reduced();
	void set_finished();
	void set_local_predecessor(OrderedReducerView<Monoid>* pred);
	void set_predecessor(OrderedReducerView<Monoid>* pred);
	template <typename ... PutParams>
	void add_data(PutParams&& ... params);
	typename Monoid::OutputType get_data();
private:
	typedef OrderedReducerView<Monoid> View;

	Monoid data;
	OrderedReducerView<Monoid>* local_pred;
	OrderedReducerView<Monoid>* pred;
	OrderedReducerView<Monoid>* reuse;
	// Bit 1: has a child, Bit 2: has been used, Bit 3: has been connected to a non-local parent (Bit 0 isn't used any more)
	uint8_t state;
};

template <class Monoid>
template <typename ... ConsParams>
OrderedReducerView<Monoid>::OrderedReducerView(ConsParams&& ... params)
:data(std::forward<ConsParams&&>(params) ...), local_pred(NULL), pred(NULL), reuse(NULL), state(0x0u) {

}

template <class Monoid>
OrderedReducerView<Monoid>::OrderedReducerView(Monoid const& template_monoid)
:data(template_monoid), local_pred(NULL), pred(NULL), reuse(NULL), state(0x2u) {

}

template <class Monoid>
OrderedReducerView<Monoid>::~OrderedReducerView() {
	if(reuse != NULL) {
		delete reuse;
	}
}

template <class Monoid>
OrderedReducerView<Monoid>* OrderedReducerView<Monoid>::fold() {
	OrderedReducerView<Monoid>* ret = this;
	while((ret->state & 0x4) == 0x0 && ret->local_pred != NULL) {
		OrderedReducerView<Monoid>* tmp = ret->local_pred;
		if(ret->reuse != NULL) {
			delete ret->reuse;
		}
		ret->reuse = tmp->reuse;
		tmp->reuse = ret;
	//	delete ret;
		ret = tmp;
	}
/*	if(ret->pred != NULL) {
		ret->pred->reduce();
	}*/
	return ret;


/*	OrderedReducerView<Monoid>* ret = this;
	while((ret->state & 0x4) == 0 && ret->pred != NULL) {
		OrderedReducerView<Monoid>* tmp = ret->pred;
		while(tmp->reuse != NULL) {
			tmp = tmp->reuse;
		}
		tmp->reuse = ret;
		ret = ret->pred;
	}
	if(ret->pred != NULL) {
		ret->pred->reduce();
	}
	return ret;*/

	// refactored as the previous implementation seemed to generate too long lists of reuse views
/*	if((state & 0x4) == 0 && pred != NULL) {
		OrderedReducerView<Monoid>* tmp1 = this;
		OrderedReducerView<Monoid>* tmp2 = pred;
		while((tmp2->state & 0x4) == 0 && tmp2->pred != NULL){
			if(tmp1->reuse != NULL) {
				delete tmp1->reuse;
			}
			tmp1->reuse = tmp2;

			tmp1 = tmp2;
			tmp2 = tmp2->pred;
		}

		if(tmp1->reuse != NULL) {
			delete tmp1->reuse;
		}
		tmp1->reuse = tmp2->reuse;
		tmp2->reuse = this;
		return tmp2;
	}
	else if(pred != NULL) {
		pred->reduce();
	}
	return this;*/
	// refactored as the previous implementation seemed to generate too long lists of reuse views
/*	if((state & 0x4) == 0 && pred != NULL) {
		OrderedReducerView<Monoid>* tmp = pred;

		// Delete intermediate foldable elements
		// Do not reuse them. Reuse is primarily for the sequential case.
		// If we have to fold more than once, we are not sequential any more
		while((tmp->state & 0x4) == 0 && tmp->pred != NULL){
			OrderedReducerView<Monoid>* tmp2 = tmp->pred;
			delete tmp;
			tmp = tmp2;
		}

		if(reuse != NULL) {
			delete reuse;
		}
		reuse = tmp->reuse;
		tmp->reuse = this;
		return tmp;
	}
	else if(pred != NULL) {
		pred->reduce();
	}
	return this;*/
}

template <class Monoid>
OrderedReducerView<Monoid>* OrderedReducerView<Monoid>::create_parent_view() {
	OrderedReducerView<Monoid>* ret = reuse;
	if(ret != NULL) {
		reuse = ret->reuse;
		ret->data.reset();
		ret->local_pred = NULL;
		ret->pred = NULL;
	//	ret->parent = NULL;
		ret->reuse = NULL;
		ret->state = 0x2;
	}
	else {
		ret = new View(static_cast<Monoid const&>(data));
		pheet_assert(ret->state == 0x2);
	}
	return ret;
}

template <class Monoid>
void OrderedReducerView<Monoid>::reduce(bool allow_local) {
	// TODO: Check if we can't just always use a right reduction and keep the predecessor view while dropping the local one. This might simplify reducers a lot
	if(local_pred != NULL) {
		if(allow_local) {
			while(local_pred->local_pred != NULL) {
				data.left_reduce(local_pred->data);
				View* tmp = local_pred->local_pred;
				delete local_pred;
				local_pred = tmp;
			}
			if(local_pred->pred != NULL) {
				data.left_reduce(local_pred->data);
				View* tmp = local_pred->pred;
				delete local_pred;
				local_pred = NULL;
				pred = tmp;
			}
			else if((local_pred->state & 0x2) == 0x0) {
				data.left_reduce(local_pred->data);
				delete local_pred;
				local_pred = NULL;
				// No more children
				state &= 0xFD;
			}
		}
		else {
			pred = local_pred;
			local_pred = nullptr;
		}
	}
	// TODO: manual unrolling to reduce number of fences used (a single fence should be enough,
	// as long as we know all pointers have been read before the fence.)
	if(pred != NULL) {
		pheet_assert(local_pred == NULL);
		state |= 0x4;
		state |= 0x8; // We cannot be sure that we are local anymore
		bool active;
		do {
			active = false;
			while(pred->pred != NULL) {
				MEMORY_FENCE();
				data.left_reduce(pred->data);
				View* tmp = pred->pred;
				delete pred;
				pred = tmp;
				active = true;
			}
			while(pred->local_pred != NULL) {
				MEMORY_FENCE();
				data.left_reduce(pred->data);
				View* tmp = pred->local_pred;
				delete pred;
				pred = tmp;
				active = true;
			}
		}while(active);
		if((pred->state & 0x2) == 0x0) {
			MEMORY_FENCE();
			data.left_reduce(pred->data);
			delete pred;
			pred = NULL;
			// No more children
			state &= 0xFD;
		}
	}

/*
	if(pred != NULL && (pred->pred != NULL || (pred->state & 0x2) == 0x0)) {
		bool local = (state & 0x1) == 0x1;
		// Definitely not waiting for a child any more. and maybe not local any more
		state &= 0xFC;

		do {
			if(!local) {
				MEMORY_FENCE();
			}
			if((pred->state & 0x4) == 0x4) {
				data.left_reduce(pred->data);
				state |= 0x4;
			}
			local = local && ((pred->state & 0x1) == 0x1);
			View* tmp = pred->pred;
			// No memory reclamation as this view is from another fork
			delete pred;
			pred = tmp;
		}while(pred != NULL && (pred->pred != NULL || (pred->state & 0x2) == 0x0));
*/
	/*	if(local) {
			state |= 0x1;
		}
		else {*/
	//		state &= 0xFE;
		//}
//	}
}

template <class Monoid>
bool OrderedReducerView<Monoid>::is_reduced() {
	return pred == NULL && (state & 0x2) == 0;
}

template <class Monoid>
void OrderedReducerView<Monoid>::set_local_predecessor(OrderedReducerView<Monoid>* local_pred) {
	pheet_assert(this->pred == NULL);
	pheet_assert(this->local_pred == NULL);
	pheet_assert(local_pred != this);
	if((state & 0x8) == 0x8) {
		// This element has already been connected to some non-local element.
		// Therefore we have to treat it like something non-local
		this->pred = local_pred;
	}
	else {
		this->local_pred = local_pred;
	}
}

template <class Monoid>
void OrderedReducerView<Monoid>::set_predecessor(OrderedReducerView<Monoid>* pred) {
	pheet_assert(this->local_pred == NULL);
	pheet_assert(this->pred == NULL);
	pheet_assert(pred != this);
	// Warn other local elements that this element is connected to something non-local
	// OrderedReducer takes care that local elements are folded before the call to set_predecessor
	// As local elements live in a sequential world we are guaranteed to have them either folded now, or
	// warned if they are connected later
//	state |= 0x8;
//	MEMORY_FENCE();
	this->pred = pred;
}

template <typename Monoid>
template <typename ... PutParams>
void OrderedReducerView<Monoid>::add_data(PutParams&& ... params) {
	state |= 0x4;
	data.put(std::forward<PutParams&&>(params) ...);
}

template <class Monoid>
typename Monoid::OutputType OrderedReducerView<Monoid>::get_data() {
	return data.get();
}

}

#endif /* ORDEREDREDUCERVIEW_H_ */
