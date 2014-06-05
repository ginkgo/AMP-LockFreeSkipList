/*
 * CopyOnWritePtr.h
 *
 *  Created on: Feb 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef COPYONWRITEPTR_H_
#define COPYONWRITEPTR_H_

#include <pheet/pheet.h>

namespace pheet {

struct CopyOnWritePtrSyncData {
	CopyOnWritePtrSyncData()
		: num_created(1), num_finished(0), parent(nullptr) {}
	CopyOnWritePtrSyncData(CopyOnWritePtrSyncData* parent)
		: num_created(1), num_finished(0), parent(parent) {}

	size_t num_created;
	size_t num_finished;
	CopyOnWritePtrSyncData* parent;
};

template <class Pheet, typename T>
class CopyOnWritePtr {
public:
	typedef CopyOnWritePtr<Pheet, T> Self;
	typedef CopyOnWritePtrSyncData SyncData;

	CopyOnWritePtr();
	CopyOnWritePtr(Self& other);
	CopyOnWritePtr(Self&& other);
	~CopyOnWritePtr();

	void set(T* data);
	T const* get();
	T* get_writable();

private:
	void free_data();

	SyncData* sync_data;
	T* data;
	bool own_sync_data;
};

template <class Pheet, typename T>
CopyOnWritePtr<Pheet, T>::CopyOnWritePtr()
: sync_data(nullptr), data(nullptr), own_sync_data(false) {

}

template <class Pheet, typename T>
CopyOnWritePtr<Pheet, T>::CopyOnWritePtr(Self& other)
: data(other.data), own_sync_data(false) {
	if(other.data != nullptr) {
		if(other.sync_data != nullptr && (other.own_sync_data || other.sync_data->num_created == other.sync_data->num_finished)) {
			other.own_sync_data = true;
			sync_data = other.sync_data;
			++(other.sync_data.num_created);
		}
		else {
			other.sync_data = new SyncData(other.sync_data);
			other.own_sync_data = true;
			sync_data = other.sync_data;
		}
	}
}

template <class Pheet, typename T>
CopyOnWritePtr<Pheet, T>::CopyOnWritePtr(Self&& other)
: sync_data(other.sync_data), data(other.data), own_sync_data(other.own_sync_data) {
	other.sync_data = nullptr;
	other.own_sync_data = false;
}

template <class Pheet, typename T>
CopyOnWritePtr<Pheet, T>::~CopyOnWritePtr() {
	free_data();
}

template <class Pheet, typename T>
void CopyOnWritePtr<Pheet, T>::free_data() {
	while(sync_data != nullptr) {
		size_t ov = SIZET_FETCH_AND_ADD(&(sync_data->num_finished), 1);
		if(ov == sync_data->num_created) {
			// This was the last one. We are responsible for cleaning up
			auto parent = sync_data->parent;
			delete sync_data;
			sync_data = parent;
		}
		else {
			break;
		}
	}
	if((sync_data == nullptr) && (data != nullptr)) {
		delete data;
		data = nullptr;
	}
	sync_data = nullptr;
}

template <class Pheet, typename T>
void CopyOnWritePtr<Pheet, T>::set(T* data) {
	free_data();
	this->data = data;
}

template <class Pheet, typename T>
T const* CopyOnWritePtr<Pheet, T>::get() {
	return data;
}

template <class Pheet, typename T>
T* CopyOnWritePtr<Pheet, T>::get_writable() {
	while(sync_data != nullptr) {
		if(sync_data->num_finished == sync_data->num_created) {
			auto parent = sync_data->parent;
			delete sync_data;
			sync_data = parent;
		}
		else {
			T* tmp = new T(data);
			free_data();
			data = tmp;
			break;
		}
	}
	return data;
}


}

#endif /* COPYONWRITEPTR_H_ */
