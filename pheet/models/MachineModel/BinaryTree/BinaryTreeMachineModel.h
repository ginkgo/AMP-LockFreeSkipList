/*
 * BinaryTreeMachineModel.h
 *
 *  Created on: Feb 1, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BINARYTREEMACHINEMODEL_H_
#define BINARYTREEMACHINEMODEL_H_

namespace pheet {

template <class Pheet, class BaseModelT>
class BinaryTreeMachineModel {
public:
	typedef BinaryTreeMachineModel<Pheet, BaseModelT> Self;
	typedef BinaryTreeMachineModel<Pheet, BaseModelT> ThisType;
	typedef BaseModelT BaseModel;

	BinaryTreeMachineModel();
	BinaryTreeMachineModel(Self const& other);
	BinaryTreeMachineModel(Self const&& other);
	~BinaryTreeMachineModel();

	ThisType& operator=(ThisType const& other);
	ThisType& operator=(ThisType const&& other);

	bool is_leaf();
	procs_t get_num_children();
	Self get_child(procs_t id);
	procs_t get_node_id();
	procs_t get_node_offset();
	procs_t get_num_leaves();
	procs_t get_memory_level();
	procs_t get_numa_memory_level();

	void bind();
	void unbind();

	template <typename T>
	bool is_partially_numa_local(T const* addr, size_t count) {
		return base.is_partially_numa_local(addr, count);
	}

	template <typename T>
	bool is_fully_numa_local(T const* addr, size_t count) {
		return base.is_fully_numa_local(addr, count);
	}

	procs_t get_data_numa_node_id(void const* addr) {
		return base.get_data_numa_node_id(addr);
	}

	procs_t get_numa_node_id() {
		return base.get_numa_node_id();
	}
private:
	BinaryTreeMachineModel(BaseModel& base, procs_t first_child, procs_t last_child);
	BaseModel base;
	procs_t first_child;
	procs_t last_child;
};

template <class Pheet, class BaseModelT>
BinaryTreeMachineModel<Pheet, BaseModelT>::BinaryTreeMachineModel()
: first_child(0), last_child(0) {
	pheet_assert(base.is_leaf() || base.get_num_children() > 0);
	while((!base.is_leaf()) && base.get_num_children() == 1) {
		base = base.get_child(0);
	}
	if(!base.is_leaf()) {
		last_child = base.get_num_children() - 1;
		pheet_assert(last_child > first_child);
	}
}

template <class Pheet, class BaseModelT>
BinaryTreeMachineModel<Pheet, BaseModelT>::BinaryTreeMachineModel(Self const& other)
: base(other.base), first_child(other.first_child), last_child(other.last_child) {
	pheet_assert(base.is_leaf() || base.get_num_children() > 1);
	pheet_assert(base.is_leaf() || last_child > first_child);
}

template <class Pheet, class BaseModelT>
BinaryTreeMachineModel<Pheet, BaseModelT>::BinaryTreeMachineModel(Self const&& other)
: base(other.base), first_child(other.first_child), last_child(other.last_child) {
	pheet_assert(base.is_leaf() || base.get_num_children() > 1);
	pheet_assert(base.is_leaf() || last_child > first_child);
}

template <class Pheet, class BaseModelT>
BinaryTreeMachineModel<Pheet, BaseModelT>::BinaryTreeMachineModel(BaseModel& base, procs_t first_child, procs_t last_child)
: base(base), first_child(first_child), last_child(last_child) {
	pheet_assert(this->base.is_leaf() || this->last_child >= this->first_child);
	if(!this->base.is_leaf() && (this->last_child == this->first_child)) {
		do{
			this->base = this->base.get_child(this->first_child);
			this->first_child = 0;
		} while((!this->base.is_leaf()) && this->base.get_num_children() == 1);
		if(this->base.is_leaf()) {
			this->last_child = 0;
		}
		else {
			this->last_child = this->base.get_num_children() - 1;
			pheet_assert(this->last_child > this->first_child);
		}
	}
}

template <class Pheet, class BaseModelT>
BinaryTreeMachineModel<Pheet, BaseModelT>::~BinaryTreeMachineModel() {

}

template <class Pheet, class BaseModelT>
BinaryTreeMachineModel<Pheet, BaseModelT>& BinaryTreeMachineModel<Pheet, BaseModelT>::operator=(BinaryTreeMachineModel<Pheet, BaseModelT> const& other) {
	base = other.base;
	first_child = other.first_child;
	last_child = other.last_child;
}

template <class Pheet, class BaseModelT>
BinaryTreeMachineModel<Pheet, BaseModelT>& BinaryTreeMachineModel<Pheet, BaseModelT>::operator=(BinaryTreeMachineModel<Pheet, BaseModelT> const&& other) {
	base = other.base;
	first_child = other.first_child;
	last_child = other.last_child;
	return *this;
}

template <class Pheet, class BaseModelT>
bool BinaryTreeMachineModel<Pheet, BaseModelT>::is_leaf() {
	return base.is_leaf();
}

template <class Pheet, class BaseModelT>
procs_t BinaryTreeMachineModel<Pheet, BaseModelT>::get_num_children() {
	pheet_assert(!is_leaf());
	return 2;
}

template <class Pheet, class BaseModelT>
BinaryTreeMachineModel<Pheet, BaseModelT> BinaryTreeMachineModel<Pheet, BaseModelT>::get_child(procs_t id) {
	pheet_assert(!is_leaf());
	pheet_assert(last_child > first_child);
	procs_t middle = first_child + ((last_child - first_child) >> 1);
	if(id == 0) {
		return Self(base, first_child, middle);
	}
	else {
		pheet_assert(id == 1);
		return Self(base, middle + 1, last_child);
	}
}
/*
template <class Pheet, class BaseModelT>
procs_t BinaryTreeMachineModel<Pheet, BaseModelT>::get_node_id() {
	return node_id;
}*/

template <class Pheet, class BaseModelT>
procs_t BinaryTreeMachineModel<Pheet, BaseModelT>::get_node_offset() {
	if(first_child == 0) {
		return base.get_node_offset();
	}
	else {
		pheet_assert(!is_leaf());
		return base.get_child(first_child).get_node_offset();
	}
}

template <class Pheet, class BaseModelT>
procs_t BinaryTreeMachineModel<Pheet, BaseModelT>::get_num_leaves() {
	if(base.is_leaf()) {
		return base.get_num_leaves();
	}
	else if(first_child == 0 && last_child == (base.get_num_children() - 1)) {
		return base.get_num_leaves();
	}
	else {
		pheet_assert(last_child > first_child);
		procs_t ret = 0;
		for(procs_t i = first_child; i <= last_child; ++i) {
			ret += base.get_child(i).get_num_leaves();
		}
		return ret;
	}
}

template <class Pheet, class BaseModelT>
void BinaryTreeMachineModel<Pheet, BaseModelT>::bind() {
	if(base.is_leaf() || (first_child == 0 && last_child == (base.get_num_children() - 1))) {
//		pheet_assert(base.is_leaf());
		base.bind();
	}
	else {
		get_child(0).bind();
	}
}

template <class Pheet, class BaseModelT>
void BinaryTreeMachineModel<Pheet, BaseModelT>::unbind() {
	if(base.is_leaf() || (first_child == 0 && last_child == (base.get_num_children() - 1))) {
//		pheet_assert(base.is_leaf());
		base.unbind();
	}
	else {
		get_child(0).unbind();
	}
}

template <class Pheet, class BaseModelT>
procs_t BinaryTreeMachineModel<Pheet, BaseModelT>::get_memory_level() {
	return base.get_memory_level();
}

template <class Pheet, class BaseModelT>
procs_t BinaryTreeMachineModel<Pheet, BaseModelT>::get_numa_memory_level() {
	return base.get_numa_memory_level();
}

}

#endif /* BINARYTREEMACHINEMODEL_H_ */
