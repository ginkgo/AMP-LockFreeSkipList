/*
 * StrategyScheduler2TaskStorageItem.h
 *
 *  Created on: Aug 12, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYSCHEDULER2TASKSTORAGEITEM_H_
#define STRATEGYSCHEDULER2TASKSTORAGEITEM_H_

namespace pheet {

template <class Pheet, typename Task, typename StackElement>
struct StrategyScheduler2TaskStorageItem {
	typedef StrategyScheduler2TaskStorageItem<Pheet, Task, StackElement> Self;

	StrategyScheduler2TaskStorageItem();

	/*
	 * Drop item without ever executing it. Can be used to implement removal of dead tasks
	 */
	void drop_item() {
		Pheet::get_place()->drop_item(this);
	}

	Task* task;
	StackElement* stack_element;

	bool operator==(Self const& other) const;
	bool operator!=(Self const& other) const;
};


template <class Pheet, typename Task, typename StackElement>
StrategyScheduler2TaskStorageItem<Pheet, Task, StackElement>::StrategyScheduler2TaskStorageItem()
: task(NULL), stack_element(NULL) {

}

template <class Pheet, typename Task, typename StackElement>
bool StrategyScheduler2TaskStorageItem<Pheet, Task, StackElement>::operator==(Self const& other) const {
	return other.task == task;
}

template <class Pheet, typename Task, typename StackElement>
bool StrategyScheduler2TaskStorageItem<Pheet, Task, StackElement>::operator!=(Self const& other) const {
	return other.task != task;
}


template <class Pheet, typename Task, typename StackElement>
class nullable_traits<StrategyScheduler2TaskStorageItem<Pheet, Task, StackElement> > {
public:
	static StrategyScheduler2TaskStorageItem<Pheet, Task, StackElement> const null_value;
};

template <class Pheet, typename Task, typename StackElement>
StrategyScheduler2TaskStorageItem<Pheet, Task, StackElement> const nullable_traits<StrategyScheduler2TaskStorageItem<Pheet, Task, StackElement> >::null_value;


}

#endif /* STRATEGYSCHEDULER2TASKSTORAGEITEM_H_ */
