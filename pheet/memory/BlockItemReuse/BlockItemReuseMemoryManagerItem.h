/*
 * ItemReuseMemoryManagerItem.h
 *
 *  Created on: July 30, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef BLOCKITEMREUSEMEMORYMANAGERITEM_H_
#define BLOCKITEMREUSEMEMORYMANAGERITEM_H_

namespace pheet {

template <class Pheet, typename T, size_t BlockSize>
class BlockItemReuseMemoryManagerItem {
public:
	typedef BlockItemReuseMemoryManagerItem<Pheet, T, BlockSize> Self;

	T items[BlockSize];
	Self* next;
};

} /* namespace pheet */
#endif /* BLOCKITEMREUSEMEMORYMANAGERITEM_H_ */
