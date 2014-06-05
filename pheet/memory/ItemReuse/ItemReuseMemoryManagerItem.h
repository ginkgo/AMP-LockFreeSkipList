/*
 * ItemReuseMemoryManagerItem.h
 *
 *  Created on: May 22, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef ITEMREUSEMEMORYMANAGERITEM_H_
#define ITEMREUSEMEMORYMANAGERITEM_H_

namespace pheet {

template <class Pheet, typename T>
class ItemReuseMemoryManagerItem {
public:
	typedef ItemReuseMemoryManagerItem<Pheet, T> Self;

	T item;
	Self* next;
};

} /* namespace pheet */
#endif /* ITEMREUSEMEMORYMANAGERITEM_H_ */
