/*
 * FrameMemoryManager.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FRAMEMEMORYMANAGER_H_
#define FRAMEMEMORYMANAGER_H_

#include <pheet/memory/BlockItemReuse/BlockItemReuseMemoryManager.h>

#include "FrameMemoryManagerItem.h"
#include "FrameMemoryManagerFrame.h"
#include "FrameMemoryManagerSingleton.h"
#include "FrameMemoryManagerPtr.h"

namespace pheet {

template <class Pheet, typename T, class ReuseCheck, size_t FrameGranularity>
class FrameMemoryManagerImpl {
public:
	typedef FrameMemoryManagerSingleton<Pheet> Singleton;
	typedef FrameMemoryManagerItem<Pheet, T, ReuseCheck> Item;
	typedef FrameMemoryManagerFrame<Pheet> Frame;
	typedef FrameMemoryManagerPtr<Pheet, Item> PtrType;

	typedef BlockItemReuseMemoryManager<Pheet, Item, FrameMemoryManagerItemReuseCheck<Item, ReuseCheck> > ItemMemoryManager;

	template <size_t NewFrameGranularity>
	using WithFrameGranularity = FrameMemoryManagerImpl<Pheet, NewFrameGranularity>;

	FrameMemoryManagerImpl()
	: singleton(Pheet::template place_singleton<Singleton>()),
	  current_frame(singleton.next_frame),
	  processed(0) {}

	~FrameMemoryManagerImpl() {}

	PtrType acquire_item() {
		Item& item = item_mm.acquire_item();
		item.phase.store(-1, std::memory_order_release);
		item.current_frame.store(current_frame, std::memory_order_release);

		PtrType ret(&item);
		return ret;
	}

private:
	Singleton& singleton;
	Frame* current_frame;
	size_t processed;

	ItemMemoryManager item_mm;
};

template <class Pheet>
using FrameMemoryManager = FrameMemoryManagerImpl<Pheet, 256>;

} /* namespace pheet */
#endif /* FRAMEMEMORYMANAGER_H_ */
