/*
 * FinisherSchedulerFinishRegion.h
 *
 *  Created on: May 23, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FINISHERSCHEDULERFINISHREGION_H_
#define FINISHERSCHEDULERFINISHREGION_H_

namespace pheet {

template <class Pheet>
class FinisherSchedulerFinishRegion {
public:
	typedef typename Pheet::Primitives::Finisher Finisher;
	typedef typename Pheet::Environment::Place Place;

	FinisherSchedulerFinishRegion()
	:fin(std::move(Pheet::get_place()->current_finisher)){
		Pheet::get_place()->current_finisher.activate();
	}
	~FinisherSchedulerFinishRegion() {
		Place* p = Pheet::get_place();
		p->wait_for_finish(std::move(p->current_finisher));
		p->current_finisher = std::move(fin);
	}

private:
	Finisher fin;
};

} /* namespace pheet */
#endif /* FINISHERSCHEDULERFINISHREGION_H_ */
