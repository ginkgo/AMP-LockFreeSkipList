/*
 * HWLocMachineModel.h
 *
 *  Created on: Jan 31, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef HWLOCMACHINEMODEL_H_
#define HWLOCMACHINEMODEL_H_

#include "../../../settings.h"

#include <hwloc.h>
#include <iostream>
#include <errno.h>


namespace pheet {

template <class Pheet>
class HWLocTopologyInfo {
public:
	HWLocTopologyInfo();
	~HWLocTopologyInfo();

	hwloc_obj_t get_root_obj();
	unsigned int get_root_depth();
	unsigned int get_numa_depth();
	unsigned int get_total_depth();
	unsigned int get_total_width();

	hwloc_cpuset_t get_binding();
	void bind(hwloc_cpuset_t cpus);
	void free_binding(hwloc_cpuset_t cpus);

	/*
	 * This does not seem to be supported under linux, so use with care
	 */
	template <typename T>
	bool is_partially_numa_local(hwloc_obj_t node, T const* addr, size_t count) {
		hwloc_nodeset_t ns = hwloc_bitmap_alloc();
 		hwloc_membind_policy_t p;
		hwloc_get_area_membind_nodeset(topology, addr, sizeof(T)*count, ns, &p, 0);

		int ret = hwloc_bitmap_isincluded(node->nodeset, ns);
		hwloc_bitmap_free(ns);
		return ret != 0;
	}

	/*
	 * This does not seem to be supported under linux, so use with care
	 */
	template <typename T>
	bool is_fully_numa_local(hwloc_obj_t node, T const* addr, size_t count) {
		hwloc_nodeset_t ns = hwloc_bitmap_alloc();
		hwloc_membind_policy_t p;
		int ret = hwloc_get_area_membind_nodeset(topology, addr, sizeof(T)*count, ns, &p, HWLOC_MEMBIND_STRICT);
		if(ret == -1) {
			hwloc_bitmap_free(ns);
			return false;
		}
		ret = hwloc_bitmap_isequal(node->nodeset, ns);
		hwloc_bitmap_free(ns);
		return ret > 0;
	}

	/*
	 * This does not seem to be supported under linux, so use with care
	 */
	procs_t get_numa_node_id(hwloc_obj_t node, void const* addr) {
		hwloc_nodeset_t ns = hwloc_bitmap_alloc();
		hwloc_membind_policy_t p;
		int ret = hwloc_get_area_membind_nodeset(topology, addr, 1, ns, &p, 0);
		if(ret == -1) {
			if(errno == ENOSYS) {
				throw -1;
			}
		}

		int first = hwloc_bitmap_first(ns);
		hwloc_bitmap_free(ns);

		if(first < 0) {
			return std::numeric_limits<procs_t>::max();
		}
		return static_cast<procs_t>(first);
	}
private:
	HWLocTopologyInfo(HWLocTopologyInfo* topo, int depth);

	hwloc_topology_t topology;
	unsigned int root_depth;
	unsigned int numa_depth;
	unsigned int total_depth;
};

template <class Pheet>
HWLocTopologyInfo<Pheet>::HWLocTopologyInfo() {
	/* Allocate and initialize topology object. */
	hwloc_topology_init(&topology);

	/* Perform the topology detection. */
	hwloc_topology_load(topology);

	root_depth = hwloc_get_root_obj(topology)->depth;
	numa_depth = hwloc_get_type_or_above_depth(topology, HWLOC_OBJ_NODE);
	total_depth = hwloc_get_type_or_above_depth(topology, HWLOC_OBJ_CORE);
}

template <class Pheet>
HWLocTopologyInfo<Pheet>::~HWLocTopologyInfo() {
	hwloc_topology_destroy(topology);
}

template <class Pheet>
hwloc_obj_t HWLocTopologyInfo<Pheet>::get_root_obj() {
	return hwloc_get_root_obj(topology);
}

template <class Pheet>
unsigned int HWLocTopologyInfo<Pheet>::get_root_depth() {
	return root_depth;
}

template <class Pheet>
unsigned int HWLocTopologyInfo<Pheet>::get_numa_depth() {
	return numa_depth;
}

template <class Pheet>
unsigned int HWLocTopologyInfo<Pheet>::get_total_depth() {
	return total_depth;
}

template <class Pheet>
unsigned int HWLocTopologyInfo<Pheet>::get_total_width() {
	return hwloc_get_nbobjs_by_depth(topology, total_depth);
}

template <class Pheet>
hwloc_cpuset_t HWLocTopologyInfo<Pheet>::get_binding() {
	hwloc_cpuset_t set = hwloc_bitmap_alloc();
	hwloc_get_cpubind(topology, set, HWLOC_CPUBIND_THREAD);
	return set;
}

template <class Pheet>
void HWLocTopologyInfo<Pheet>::bind(hwloc_cpuset_t cpus) {
/*	int x = hwloc_bitmap_first(cpus);
	while(x != -1) {
		if(hwloc_bitmap_isset(cpus, x))
			std::cout << x << " ";
		x = hwloc_bitmap_next(cpus, x);
	}*/
	hwloc_set_cpubind(topology, cpus, HWLOC_CPUBIND_THREAD);
/*	x = hwloc_bitmap_first(cpus);
	while(x != -1) {
		if(hwloc_bitmap_isset(cpus, x))
			std::cout << x << " ";
		x = hwloc_bitmap_next(cpus, x);
	}
	std::cout << std::endl;*/
//	std::cout << ret << " " << hwloc_bitmap_first(cpus) << " " << hwloc_bitmap_next(cpus, hwloc_bitmap_first(cpus)) << std::endl;
}

template <class Pheet>
void HWLocTopologyInfo<Pheet>::free_binding(hwloc_cpuset_t cpus) {
	hwloc_bitmap_free(cpus);
}

template <class Pheet>
class HWLocMachineModel {
public:
	typedef HWLocMachineModel<Pheet> ThisType;

	HWLocMachineModel();
	HWLocMachineModel(HWLocMachineModel<Pheet> const& other);
	HWLocMachineModel(HWLocMachineModel<Pheet> const&& other);
	~HWLocMachineModel();

	ThisType& operator=(ThisType const& other);
	ThisType& operator=(ThisType const&& other);

	bool is_leaf();
	procs_t get_num_children();
	HWLocMachineModel<Pheet> get_child(procs_t id);
	procs_t get_node_offset();
	procs_t get_last_leaf_offset();
//	procs_t get_node_id();
	procs_t get_num_leaves();
	procs_t get_memory_level();
	procs_t get_numa_memory_level();

	bool supports_SMT() const { return false; }
 
	void bind();
	void unbind();

	/*
	 * This does not seem to be supported under linux, so use with care
	 */
	template <typename T>
	bool is_partially_numa_local(T const* addr, size_t count) {
		return topo->is_partially_numa_local(node, addr, count);
	}

	/*
	 * This does not seem to be supported under linux, so use with care
	 */
	template <typename T>
	bool is_fully_numa_local(T const* addr, size_t count) {
		return topo->is_fully_numa_local(node, addr, count);
	}

	/*
	 * This does not seem to be supported under linux, so use with care
	 */
	procs_t get_data_numa_node_id(void const* addr) {
		return topo->get_numa_node_id(node, addr);
	}

	procs_t get_numa_node_id() {
		int first = hwloc_bitmap_first(node->nodeset);
		if(first < 0)
			return std::numeric_limits<procs_t>::max();
		return static_cast<procs_t>(first);
	}
private:
	HWLocMachineModel(HWLocTopologyInfo<Pheet>* topo, hwloc_obj_t node);
	HWLocTopologyInfo<Pheet>* topo;
	hwloc_obj_t node;
	bool root;

	hwloc_cpuset_t prev_binding;
#ifdef PHEET_DEBUG_MODE
	bool bound;
#endif
};

template <class Pheet>
HWLocMachineModel<Pheet>::HWLocMachineModel()
: topo(new HWLocTopologyInfo<Pheet>()), node(topo->get_root_obj()), root(true), prev_binding(nullptr) {
#ifdef PHEET_DEBUG_MODE
	bound = false;
#endif
}

template <class Pheet>
HWLocMachineModel<Pheet>::HWLocMachineModel(HWLocMachineModel<Pheet> const& other)
: topo(other.topo), node(other.node), root(false), prev_binding(nullptr) {
#ifdef PHEET_DEBUG_MODE
	bound = false;
#endif
}

template <class Pheet>
HWLocMachineModel<Pheet>::HWLocMachineModel(HWLocMachineModel<Pheet> const&& other)
: topo(other.topo), node(other.node), root(false), prev_binding(nullptr) {
#ifdef PHEET_DEBUG_MODE
	bound = false;
#endif
}

template <class Pheet>
HWLocMachineModel<Pheet>::HWLocMachineModel(HWLocTopologyInfo<Pheet>* topo, hwloc_obj_t node)
: topo(topo), node(node), root(false), prev_binding(nullptr) {
#ifdef PHEET_DEBUG_MODE
	bound = false;
#endif
}

template <class Pheet>
HWLocMachineModel<Pheet>::~HWLocMachineModel() {
	if(root) {
		delete topo;
	}
	if(prev_binding != nullptr) {
		topo->free_binding(prev_binding);
	}
}

template <class Pheet>
HWLocMachineModel<Pheet>& HWLocMachineModel<Pheet>::operator=(HWLocMachineModel<Pheet> const& other) {
	pheet_assert(topo == other.topo || !root);
	topo = other.topo;
	node = other.node;
	return *this;
}

template <class Pheet>
HWLocMachineModel<Pheet>& HWLocMachineModel<Pheet>::operator=(HWLocMachineModel<Pheet> const&& other) {
	pheet_assert(topo == other.topo || !root);
	topo = other.topo;
	node = other.node;
	return *this;
}

template <class Pheet>
procs_t HWLocMachineModel<Pheet>::get_num_children() {
	return node->arity;
}

template <class Pheet>
HWLocMachineModel<Pheet> HWLocMachineModel<Pheet>::get_child(procs_t id) {
	pheet_assert(id < node->arity);
	pheet_assert((node->depth) < (topo->get_total_depth()));
	return HWLocMachineModel(topo, node->children[id]);
}

template <class Pheet>
bool HWLocMachineModel<Pheet>::is_leaf() {
	return (node->depth) >= (topo->get_total_depth());
}

template <class Pheet>
void HWLocMachineModel<Pheet>::bind() {
#ifdef PHEET_DEBUG_MODE
	pheet_assert(!bound);
	bound = true;
#endif
	prev_binding = topo->get_binding();
	topo->bind(node->cpuset);
}

template <class Pheet>
void HWLocMachineModel<Pheet>::unbind() {
#ifdef PHEET_DEBUG_MODE
	pheet_assert(bound);
	bound = false;
#endif
	topo->bind(prev_binding);
}

template <class Pheet>
procs_t HWLocMachineModel<Pheet>::get_node_offset() {
	if(!is_leaf()) {
		return get_child(0).get_node_offset();
	}
	return node->logical_index;
}

template <class Pheet>
procs_t HWLocMachineModel<Pheet>::get_last_leaf_offset() {
	if(!is_leaf()) {
		return get_child(get_num_children() - 1).get_last_leaf_offset();
	}
	return node->logical_index;
}
/*
template <class Pheet>
procs_t HWLocMachineModel<Pheet>::get_node_id() {
	return node->logical_index;
}*/

template <class Pheet>
procs_t HWLocMachineModel<Pheet>::get_num_leaves() {
	if(node->depth == (topo->get_total_depth())) {
		return 1;
	}
	else if(node->depth == topo->get_root_depth()) {
		return topo->get_total_width();
	}
	else {
		return (get_child(get_num_children() - 1).get_last_leaf_offset() + 1) - get_node_offset();
	}
}


template <class Pheet>
procs_t HWLocMachineModel<Pheet>::get_memory_level() {
	return node->depth;
}

template <class Pheet>
procs_t HWLocMachineModel<Pheet>::get_numa_memory_level() {
	return std::min(node->depth, topo->get_numa_depth());
}

}

#endif /* HWLOCMACHINEMODEL_H_ */
