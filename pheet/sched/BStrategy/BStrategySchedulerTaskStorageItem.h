/*
 * BStrategySchedulerTaskStorageItem.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BSTRATEGYSCHEDULERTASKSTORAGEITEM_H_
#define BSTRATEGYSCHEDULERTASKSTORAGEITEM_H_

namespace pheet {

template <class Pheet, typename Task, typename StackElement>
struct BStrategySchedulerTaskStorageItem {
	typedef BStrategySchedulerTaskStorageItem<Pheet, Task, StackElement> Self;

	BStrategySchedulerTaskStorageItem();

	Task* task;
	StackElement* stack_element;

	bool operator==(Self const& other) const;
	bool operator!=(Self const& other) const;
};


template <class Pheet, typename Task, typename StackElement>
BStrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::BStrategySchedulerTaskStorageItem()
: task(NULL), stack_element(NULL) {

}

template <class Pheet, typename Task, typename StackElement>
bool BStrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::operator==(Self const& other) const {
	return other.task == task;
}

template <class Pheet, typename Task, typename StackElement>
bool BStrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::operator!=(Self const& other) const {
	return other.task != task;
}


template <class Pheet, typename Task, typename StackElement>
class nullable_traits<BStrategySchedulerTaskStorageItem<Pheet, Task, StackElement> > {
public:
	static BStrategySchedulerTaskStorageItem<Pheet, Task, StackElement> const null_value;
};

template <class Pheet, typename Task, typename StackElement>
BStrategySchedulerTaskStorageItem<Pheet, Task, StackElement> const nullable_traits<BStrategySchedulerTaskStorageItem<Pheet, Task, StackElement> >::null_value;


}

#endif /* BSTRATEGYSCHEDULERTASKSTORAGEITEM_H_ */
