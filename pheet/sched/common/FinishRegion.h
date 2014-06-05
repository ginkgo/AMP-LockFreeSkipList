/*
 * Finish.h
 *
 *  Created on: 26.07.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef FINISH_H_
#define FINISH_H_

/*
 *
 */
namespace pheet {

template <class Pheet>
class FinishRegion {
public:
	FinishRegion();
	~FinishRegion();

private:
};

template <class Pheet>
FinishRegion<Pheet>::FinishRegion() {
	Pheet::get_place()->start_finish_region();
}

template <class Pheet>
FinishRegion<Pheet>::~FinishRegion() {
	Pheet::get_place()->end_finish_region();
}

}

#endif /* FINISH_H_ */
