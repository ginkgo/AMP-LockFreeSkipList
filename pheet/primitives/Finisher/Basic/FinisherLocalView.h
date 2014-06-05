/*
 * FinisherLocalView.h
 *
 *  Created on: May 22, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FINISHERLOCALVIEW_H_
#define FINISHERLOCALVIEW_H_

namespace pheet {

template <class LocalView>
struct FinisherLocalViewReuseCheck {
	bool operator()(LocalView& lv) {
		return (lv.flag & 1) == 1;
	}
};

template <class Pheet>
class FinisherLocalView {
public:
	typedef FinisherLocalView<Pheet> Self;
	typedef FinisherLocalViewReuseCheck<Self> ReuseCheck;

	FinisherLocalView()
	: p(Pheet::get_place())/*, parent(nullptr), local(1), remote(0)*/, flag(1) {}
	~FinisherLocalView() {}

	typename Pheet::Environment::Place* p;
	Self* parent;
	size_t local;
	size_t remote;
	size_t flag;
};

} /* namespace pheet */
#endif /* FINISHERLOCALVIEW_H_ */
