/*
 * FrameMemoryManagerFrameLocalView.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FRAMEMEMORYMANAGERFRAMELOCALVIEW_H_
#define FRAMEMEMORYMANAGERFRAMELOCALVIEW_H_

#include "FrameMemoryManagerFrame.h"

namespace pheet {

template <class Pheet>
class FrameMemoryManagerFrameLocalView {
public:
	typedef FrameMemoryManagerFrame<Pheet> Frame;

	FrameMemoryManagerFrameLocalView() {
		reg[0] = 0;
		reg[1] = 0;
	}
	~FrameMemoryManagerFrameLocalView() {}

	bool try_reg(Frame* frame, size_t phase) {
		size_t p = phase & 1;
		if(reg[p] == 0) {
			if(!frame->try_register_place(phase)) {
				return false;
			}
		}
		++reg[p];
		return true;
	}

	void rem_reg(Frame* frame, size_t phase) {
		size_t p = phase & 1;
		pheet_assert(reg[p] > 0);
		--reg[p];
		if(reg[p] == 0) {
			frame->deregister_place(phase);
		}
	}

	bool empty() {
		return reg[0] == 0 && reg[1] == 0;
	}

	bool is_last(size_t phase) {
		pheet_assert(reg[phase & 1] > 0);
		return reg[phase & 1] == 1;
	}
private:
	size_t reg[2];
};

} /* namespace pheet */
#endif /* FRAMEMEMORYMANAGERFRAMELOCALVIEW_H_ */
