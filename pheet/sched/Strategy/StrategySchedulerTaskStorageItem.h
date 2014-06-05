/*
 * StrategySchedulerTaskStorageItem.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYSCHEDULERTASKSTORAGEITEM_H_
#define STRATEGYSCHEDULERTASKSTORAGEITEM_H_

namespace pheet {

template <class Pheet, typename Task, typename StackElement>
struct StrategySchedulerTaskStorageItem {
	typedef StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> Self;

	StrategySchedulerTaskStorageItem();

	Task* task;
	StackElement* stack_element;

	bool operator==(Self const& other) const;
	bool operator!=(Self const& other) const;
};


template <class Pheet, typename Task, typename StackElement>
StrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::StrategySchedulerTaskStorageItem()
: task(NULL), stack_element(NULL) {

}

template <class Pheet, typename Task, typename StackElement>
bool StrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::operator==(Self const& other) const {
	return other.task == task;
}

template <class Pheet, typename Task, typename StackElement>
bool StrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::operator!=(Self const& other) const {
	return other.task != task;
}


template <class Pheet, typename Task, typename StackElement>
class nullable_traits<StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> > {
public:
	static StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> const null_value;
};

template <class Pheet, typename Task, typename StackElement>
StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> const nullable_traits<StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> >::null_value;


}

#endif /* STRATEGYSCHEDULERTASKSTORAGEITEM_H_ */
