/*
 * FrameMemoryManagerPtr.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FRAMEMEMORYMANAGERPTR_H_
#define FRAMEMEMORYMANAGERPTR_H_

#include <atomic>

#include "FrameMemoryManagerFrame.h"
#include "FrameMemoryManagerFrameLocalView.h"
#include "FrameMemoryManagerSingleton.h"

namespace pheet {

template <class Pheet, class Item>
class FrameMemoryManagerPtr {
public:
	typedef FrameMemoryManagerPtr<Pheet, Item> Self;

	typedef FrameMemoryManagerFrame<Pheet> Frame;
	typedef FrameMemoryManagerFrameLocalView<Pheet> LV;
	typedef FrameMemoryManagerSingleton<Pheet> Singleton;

	FrameMemoryManagerPtr(Item* _item)
	: item(_item) {
		pheet_assert(item != nullptr);
		pheet_assert(_item->phase.load(std::memory_order_relaxed) == -1);

		Frame* frame = item->frame.load(std::memory_order_relaxed);
		Pheet::template place_singleton<Singleton>().reg(frame, phase);
	}

	FrameMemoryManagerPtr(Self const& other)
	: item(other.item.load(std::memory_order_relaxed)) {
		Item* it = item.load(std::memory_order_relaxed);
		Frame* frame = it->frame.load(std::memory_order_relaxed);
		Singleton& s = Pheet::template place_singleton<Singleton>();
		pheet_assert(frame != nullptr);
		if(!s.try_reg(frame, phase)) {
			item.store(nullptr, std::memory_order_relaxed);
		}
		if(frame != it->frame.load(std::memory_order_relaxed)) {
			s.rem_ref(frame, phase);
			item.store(nullptr, std::memory_order_relaxed);
		}
	}

	~FrameMemoryManagerPtr() {
		Frame* frame = item.load(std::memory_order_relaxed)->frame.load(std::memory_order_relaxed);
		Pheet::template place_singleton<Singleton>().rem_reg(item->frame.load(std::memory_order_relaxed), phase);
	}

	/*
	 * For efficiency reasons, no restrictions on which thread can use this method.
	 * But if not accessed by owner of pointer, accessing the item is probably unsafe
	 */
	typename Item::T* ptr() {
		return &(item.load(std::memory_order_relaxed)->data);
	}

	bool valid() {
		return item != nullptr;
	}

private:
	std::atomic<Item*> item;
	size_t phase;
};

} /* namespace pheet */
#endif /* FRAMEMEMORYMANAGERPTR_H_ */
