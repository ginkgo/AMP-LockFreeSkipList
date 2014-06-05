/*
 * FrameMemoryManagerItem.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FRAMEMEMORYMANAGERITEM_H_
#define FRAMEMEMORYMANAGERITEM_H_

#include <atomic>

#include "FrameMemoryManagerFrame.h"

namespace pheet {

template <class Pheet, typename TT>
struct FrameMemoryManagerItem {
public:
	typedef TT T;
	typedef FrameMemoryManagerFrame<Pheet> Frame;

	T data;
	std::atomic<Frame*> frame;
	std::atomic<ptrdiff_t> phase;
};


template <class Item, class BaseReuseCheck>
struct FrameMemoryManagerItemReuseCheck {
	bool operator() (Item const& item) const {
		if(brc(item.data)) {
			// Frame is always owned by the place that calls the reuse check, so no synchronization is needed
			pheet_assert(item.frame.load(std::memory_order_relaxed) != nullptr);
			ptrdiff_t p = item.phase.load(std::memory_order_relaxed);
			if(p == -1) {
				item.frame.load(std::memory_order_relaxed)->register_reusable();
				p = (ptrdiff_t) item.frame->get_phase();
				item.phase.store(p, std::memory_order_relaxed);
			}
			return item.frame.load(std::memory_order_relaxed)->can_reuse(p);
		}
		return false;
	}

	BaseReuseCheck brc;
};

} /* namespace pheet */
#endif /* FRAMEMEMORYMANAGERITEM_H_ */
