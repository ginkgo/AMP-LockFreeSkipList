/*
 * STLPriorityQueueWrapper.h
 *
 *  Created on: Nov 30, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STLPRIORITYQUEUEWRAPPER_H_
#define STLPRIORITYQUEUEWRAPPER_H_

#include "../../../settings.h"
#include <vector>
#include <queue>
#include <iostream>

namespace pheet {

template <class Pheet, typename TT, class Comparator = std::less<TT> >
class STLPriorityQueueWrapper {
public:
	typedef TT T;

	STLPriorityQueueWrapper();
	STLPriorityQueueWrapper(Comparator const& comp);
	~STLPriorityQueueWrapper();

	void push(T item);
	T peek();
	T pop();

	size_t get_length() const;
	bool is_empty() const;

	static void print_name();

private:
	std::priority_queue<T, std::vector<T>, Comparator> stl_pq;
};


template <class Pheet, typename TT, class Comparator>
STLPriorityQueueWrapper<Pheet, TT, Comparator>::STLPriorityQueueWrapper() {

}

template <class Pheet, typename TT, class Comparator>
STLPriorityQueueWrapper<Pheet, TT, Comparator>::STLPriorityQueueWrapper(Comparator const& comp)
: stl_pq(comp) {

}

template <class Pheet, typename TT, class Comparator>
STLPriorityQueueWrapper<Pheet, TT, Comparator>::~STLPriorityQueueWrapper() {

}

template <class Pheet, typename TT, class Comparator>
void STLPriorityQueueWrapper<Pheet, TT, Comparator>::push(T item) {
	stl_pq.push(item);
}

template <class Pheet, typename TT, class Comparator>
TT STLPriorityQueueWrapper<Pheet, TT, Comparator>::peek() {
	return stl_pq.top();
}


template <class Pheet, typename TT, class Comparator>
TT STLPriorityQueueWrapper<Pheet, TT, Comparator>::pop() {
	T ret = stl_pq.top();
	stl_pq.pop();
	return ret;
}

template <class Pheet, typename TT, class Comparator>
size_t STLPriorityQueueWrapper<Pheet, TT, Comparator>::get_length() const {
	return stl_pq.size();
}

template <class Pheet, typename TT, class Comparator>
bool STLPriorityQueueWrapper<Pheet, TT, Comparator>::is_empty() const {
	return stl_pq.empty();
}

template <class Pheet, typename TT, class Comparator>
void STLPriorityQueueWrapper<Pheet, TT, Comparator>::print_name() {
	std::cout << "STLPriorityQueueWrapper";
}

}

#endif /* STLPRIORITYQUEUEWRAPPER_H_ */
