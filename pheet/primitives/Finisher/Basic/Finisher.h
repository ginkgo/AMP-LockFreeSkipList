/*
 * Finisher.h
 *
 *  Created on: May 22, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FINISHER_H_
#define FINISHER_H_

#include <pheet/misc/atomics.h>
#include <pheet/memory/ItemReuse/ItemReuseMemoryManager.h>
#include "FinisherLocalView.h"

namespace pheet {

template <class Pheet, template <class, typename, class> class MemoryManagerT>
class FinisherImpl {
public:
	typedef FinisherImpl<Pheet, MemoryManagerT> Self;
	typedef FinisherLocalView<Pheet> LocalView;
	typedef MemoryManagerT<Pheet, LocalView, typename LocalView::ReuseCheck> MemoryManager;

	FinisherImpl();
	FinisherImpl(Self& other);
	FinisherImpl(Self&& other);
	~FinisherImpl();

	void activate();
	void deactivate();
	bool unique();
	bool locally_unique();
	void make_locally_unique();

	Self& operator=(Self& other);
	Self& operator=(Self&& other);

private:
	LocalView* get_local_view();
	LocalView* local_view;
};

template <class Pheet, template <class, typename, class> class MemoryManagerT>
FinisherImpl<Pheet, MemoryManagerT>::FinisherImpl()
: local_view(nullptr) {

}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
FinisherImpl<Pheet, MemoryManagerT>::FinisherImpl(Self& other)
: local_view(other.get_local_view()) {
	if(local_view != nullptr) {
		++(local_view->local);
	}
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
FinisherImpl<Pheet, MemoryManagerT>::FinisherImpl(Self&& other)
: local_view(other.local_view) {
	other.local_view = nullptr;
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
FinisherImpl<Pheet, MemoryManagerT>::~FinisherImpl() {
	if(local_view != nullptr) {
		deactivate();
	}
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
void FinisherImpl<Pheet, MemoryManagerT>::activate() {
	pheet_assert(local_view == nullptr);
	MemoryManager& mm = Pheet::template place_singleton<MemoryManager>();
	local_view = &(mm.acquire_item());
	++(local_view->flag);
	local_view->local = 1;
	local_view->remote = 0;
	local_view->parent = nullptr;
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
void FinisherImpl<Pheet, MemoryManagerT>::deactivate() {
	pheet_assert(local_view != nullptr);
	LocalView* v = get_local_view();
	size_t f = v->flag;
	pheet_assert((f & 1) == 0);
	MEMORY_FENCE();
	--(v->local);
	if(v->local == v->remote && SIZET_CAS(&(v->flag), f, f + 1)) {
		// No need to delete local view. Memory manager acts as a garbage collector
		// TODO: try if a local reuse list can improve performance

		// Parent can be read out here, since reuse may only happen afterwards
		// (we are cleaning up a local item)
		LocalView* p = v->parent;

		while(p != nullptr) {
			v = p;
			p = v->parent;
			f = v->flag;
			size_t r = SIZET_FETCH_AND_ADD(&(v->remote), 1);
			if(v->local != r + 1 || !SIZET_CAS(&(v->flag), f, f + 1)) {
				// No need to clean up, or clean up by other thread
				break;
			}
			// No need to delete local view (wouldn't be safe anyway)
			// Memory manager acts as a garbage collector
		}
	}
	pheet_assert(v->local >= v->remote);
	local_view = nullptr;
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
bool FinisherImpl<Pheet, MemoryManagerT>::unique() {
	LocalView* v = get_local_view();
	pheet_assert(v->local > v->remote);
	bool unique = (v->local == v->remote + 1);
	while(unique && v->parent != nullptr) {
		v = v->parent;
		unique = (v->local == v->remote + 1);
	}
	if(unique) {
		// Remove unnecessary local views
		local_view->parent = v->parent;
	}
	return unique;
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
bool FinisherImpl<Pheet, MemoryManagerT>::locally_unique() {
	LocalView* v = get_local_view();
	pheet_assert(v->local > v->remote);
	return v->local == v->remote + 1;
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
void FinisherImpl<Pheet, MemoryManagerT>::make_locally_unique() {
	LocalView* v = get_local_view();
	pheet_assert(v->local > v->remote);
	if(v->local != v->remote + 1) {
		MemoryManager& mm = Pheet::template place_singleton<MemoryManagerT>();
		local_view = &(mm.acquire_item());
		local_view->parent = v;
	}
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
typename FinisherImpl<Pheet, MemoryManagerT>::LocalView*
FinisherImpl<Pheet, MemoryManagerT>::get_local_view() {
	if(local_view->p != Pheet::get_place()) {
		LocalView* p = local_view;
		MemoryManager& mm = Pheet::template place_singleton<MemoryManager>();
		local_view = &(mm.acquire_item());
		pheet_assert((local_view->flag & 1) == 1);
		++(local_view->flag);
		local_view->local = 1;
		local_view->remote = 0;
		local_view->parent = p;
	}
	return local_view;
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
FinisherImpl<Pheet, MemoryManagerT>&
FinisherImpl<Pheet, MemoryManagerT>::operator=(Self& other) {
	if(local_view != nullptr) {
		deactivate();
	}
	local_view = other.get_local_view();
	if(local_view != nullptr) {
		++(local_view->local);
	}
	return *this;
}

template <class Pheet, template <class, typename, class> class MemoryManagerT>
FinisherImpl<Pheet, MemoryManagerT>&
FinisherImpl<Pheet, MemoryManagerT>::operator=(Self&& other) {
	if(local_view != nullptr) {
		deactivate();
	}
	local_view = other.local_view;
	other.local_view = nullptr;
	return *this;
}

template <class Pheet>
using Finisher = FinisherImpl<Pheet, ItemReuseMemoryManager>;

} /* namespace pheet */
#endif /* FINISHER_H_ */
