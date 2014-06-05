/*
 * ArrayListPrimaryTaskStorageIterator.h
 *
 *  Created on: Nov 24, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LINKEDARRAYLISTPRIMARYTASKSTORAGEITERATOR_H_
#define LINKEDARRAYLISTPRIMARYTASKSTORAGEITERATOR_H_

#include "../../../../../settings.h"

#include <limits>

namespace pheet {

template <class Storage>
class ArrayListPrimaryTaskStorageIterator {
public:
	typedef ArrayListPrimaryTaskStorageIterator<Storage> ThisType;

	ArrayListPrimaryTaskStorageIterator();
	ArrayListPrimaryTaskStorageIterator(size_t index);
	ArrayListPrimaryTaskStorageIterator(ThisType const& other);
	~ArrayListPrimaryTaskStorageIterator();

	size_t get_index() const;

	ptrdiff_t operator-(ThisType const& other) const;
	ThisType& operator++();
	ThisType& operator=(ThisType const& other);
	bool operator==(ThisType const& other) const;
	bool operator!=(ThisType const& other) const;
	bool operator<(ThisType const& other) const;

	typename Storage::Item* dereference(Storage* storage);

	void deactivate();
private:
	size_t index;
	typename Storage::ControlBlock* control_block;
	size_t control_block_item_index;
	typename Storage::ControlBlock::Item* control_block_item;
	bool ffwd;
};

/*
 * Creates an uninitialized iterator. Has to be assigned a value
 */
template <class Storage>
ArrayListPrimaryTaskStorageIterator<Storage>::ArrayListPrimaryTaskStorageIterator()
: control_block(NULL), control_block_item_index(Storage::block_size), control_block_item(NULL), ffwd(false) {

}

template <class Storage>
ArrayListPrimaryTaskStorageIterator<Storage>::ArrayListPrimaryTaskStorageIterator(size_t index)
: index(index), control_block(NULL), control_block_item_index(Storage::block_size), control_block_item(NULL), ffwd(false) {

}

template <class Storage>
inline ArrayListPrimaryTaskStorageIterator<Storage>::ArrayListPrimaryTaskStorageIterator(ThisType const& other)
: index(other.index), control_block(NULL), control_block_item_index(Storage::block_size), control_block_item(NULL), ffwd(false) {

}

template <class Storage>
ArrayListPrimaryTaskStorageIterator<Storage>::~ArrayListPrimaryTaskStorageIterator() {
	if(control_block != NULL) {
		control_block->deregister_iterator();
	}
}

template <class Storage>
inline size_t ArrayListPrimaryTaskStorageIterator<Storage>::get_index() const {
	return index;
}

template <class Storage>
inline ptrdiff_t ArrayListPrimaryTaskStorageIterator<Storage>::operator-(ThisType const& other) const {
	return index - other.index;
}

template <class Storage>
inline ArrayListPrimaryTaskStorageIterator<Storage>& ArrayListPrimaryTaskStorageIterator<Storage>::operator++() // prefix
{
	if(ffwd) {
		// Fast forward to next active block
		index = control_block->get_data()[control_block_item_index].first;
		ffwd = false;
	}
	else {
		++index;
	}
    return *this;
}

template <class Storage>
ArrayListPrimaryTaskStorageIterator<Storage>& ArrayListPrimaryTaskStorageIterator<Storage>::operator=(ThisType const& other) {
	index = other.index;
	// If we are in the same control block, no need to reset
	if(control_block_item != NULL &&
				(control_block_item_index = index - control_block_item->offset) >= Storage::block_size) {
		control_block_item = NULL;
		control_block_item_index = 0;
	}
	return *this;
}

template <class Storage>
inline bool ArrayListPrimaryTaskStorageIterator<Storage>::operator==(ThisType const& other) const {
	return index == other.index;
}

template <class Storage>
inline bool ArrayListPrimaryTaskStorageIterator<Storage>::operator!=(ThisType const& other) const {
	return index != other.index;
}

template <class Storage>
inline bool ArrayListPrimaryTaskStorageIterator<Storage>::operator<(ThisType const& other) const {
	return index < other.index;
}

template <class Storage>
typename Storage::Item* ArrayListPrimaryTaskStorageIterator<Storage>::dereference(Storage* storage) {
	size_t block_index;

	if(control_block_item != NULL &&
			(block_index = index - control_block_item->offset) < Storage::block_size) {
		// Fast path
		return control_block_item->data + block_index;
	}
	else {
		pheet_assert(storage != NULL);
		pheet_assert(storage->current_control_block != NULL);
		if(control_block != NULL && storage->current_control_block != control_block) {
			control_block->deregister_iterator();
			control_block = NULL;
		}
		if(control_block == NULL) {
			control_block = storage->acquire_control_block(); // TODO: don't forget to check inside that the pointer we acquire can satisfy the end index
			control_block_item_index = 0;
		}
		pheet_assert(index != storage->end_index);

		while(true) {
			pheet_assert(control_block_item_index < control_block->get_length());
			if(index < control_block->get_data()[control_block_item_index].first &&
					// this second condition should make it work even with wraparound (except with really extreme cases where size_t is much too small anyway)
					(control_block->get_data()[control_block_item_index].first - index < (std::numeric_limits<size_t>::max() >> 2))) {
				ffwd = true;
				return NULL;
				// Jumping in dereference is not such a good idea (iterator should always be unique) - the fast-forward (ffwd) flag is a good idea instead
				//index = control_block->get_data()[control_block_element].first;
			}
			else if((block_index = index - control_block->get_data()[control_block_item_index].offset) >= Storage::block_size) {
				++control_block_item_index;
			}
			else {
				break;
			}
		}
		control_block_item = control_block->get_data() + control_block_item_index;

		return control_block_item->data + block_index;
	}

}

template <class Storage>
void ArrayListPrimaryTaskStorageIterator<Storage>::deactivate() {
	if(control_block != NULL) {
		control_block->deregister_iterator();
	}
}

}

#endif /* LINKEDARRAYLISTPRIMARYTASKSTORAGEITERATOR_H_ */
