/*
 * FrameMemoryManagerPlaceSingleton.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FRAMEMEMORYMANAGERPLACESINGLETON_H_
#define FRAMEMEMORYMANAGERPLACESINGLETON_H_

#include <unordered_map>

#include <pheet/memory/ItemReuse/ItemReuseMemoryManager.h>
#include "FrameMemoryManagerFrame.h"
#include "FrameMemoryManagerFrameLocalView.h"

namespace pheet {

template <class Pheet, size_t BufferSize>
class FrameMemoryManagerPlaceSingletonImpl {
public:
	typedef FrameMemoryManagerFrame<Pheet> Frame;
	typedef FrameMemoryManagerFrameLocalView<Pheet> LV;

	typedef ItemReuseMemoryManager<Pheet, Frame, FrameMemoryManagerFrameReuseCheck<Frame> > FrameMemoryManager;

	FrameMemoryManagerPlaceSingletonImpl()
	: buffer_index(0) {
		for(size_t i = 0; i < BufferSize; ++i) {
			recent_regs[i] = nullptr;
		}
	}
	~FrameMemoryManagerPlaceSingletonImpl() {}

	void reg(Frame* frame, size_t& phase) {
		LV& reg = frame_regs[frame];
		phase = frame->get_phase();

		while(!reg.try_reg(frame, phase)) {
			phase = frame->get_phase();
		}
	}

	bool try_reg(Frame* frame, size_t& phase) {
		LV& reg = frame_regs[frame];
		phase = frame->get_phase();

		return reg.try_reg(frame, phase);
	}

	void rem_reg(Frame* frame, size_t phase) {
		LV& reg = frame_regs[frame];

		reg.rem_reg(frame, phase);
		if(reg.empty()) {
			frame_regs.erase(frame);
		}
	}

	/*
	 * Like rem_reg but keeps the last BufferSize accesses in a buffer to reduce traffic for cases
	 * where the same frame is accessed every time
	 */
	void rem_reg_buffered(Frame* frame, size_t phase) {
		LV& reg = frame_regs[frame];

		if(reg.is_last(phase)) {
			buffer_index = (buffer_index + 1) % BufferSize;
			if(recent_regs[buffer_index] != nullptr) {
				rem_reg(recent_regs[buffer_index], recent_reg_phases[buffer_index]);
			}
			recent_regs[buffer_index] = frame;
			recent_reg_phases[buffer_index] = phase;
		}
		else {
			reg.rem_reg(frame, phase);
			if(reg.empty()) {
				frame_regs.erase(frame);
			}
		}
	}

	Frame* next_frame() {
		return &(frames.acquire_item());
	}

private:
	FrameMemoryManager frames;
	std::unordered_map<Frame*, LV> frame_regs;

	Frame* recent_regs[BufferSize];
	size_t recent_reg_phases[BufferSize];
	size_t buffer_index;
};

template <class Pheet>
using FrameMemoryManagerPlaceSingleton = FrameMemoryManagerPlaceSingletonImpl<Pheet, 8>;

} /* namespace pheet */
#endif /* FRAMEMEMORYMANAGERPLACESINGLETON_H_ */
