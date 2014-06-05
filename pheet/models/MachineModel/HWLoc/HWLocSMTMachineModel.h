/*
 * HWLocSMTSMTMachineModel.h
 *
 *  Created on: Feb 1, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef HWLOCSMTMACHINEMODEL_H_
#define HWLOCSMTMACHINEMODEL_H_

#include "../../../settings.h"

#include <hwloc.h>


namespace pheet {

template <class Pheet>
class HWLocSMTTopologyInfo {
public:
	HWLocSMTTopologyInfo();
	~HWLocSMTTopologyInfo();

	hwloc_obj_t get_root_obj();
	unsigned int get_root_depth();
	unsigned int get_numa_depth();
	unsigned int get_total_depth();
	unsigned int get_total_width();

	hwloc_cpuset_t get_binding();
	void bind(hwloc_cpuset_t cpus);
	void free_binding(hwloc_cpuset_t cpus);

	template <typename T>
	bool is_partially_numa_local(hwloc_obj_t node, T const* addr, size_t count) {
		hwloc_nodeset_t ns = hwloc_bitmap_alloc();
 		hwloc_membind_policy_t p;
		hwloc_get_area_membind_nodeset(topology, addr, sizeof(T)*count, ns, &p, 0);

		int ret = hwloc_bitmap_isincluded(node->nodeset, ns);
		hwloc_bitmap_free(ns);
		return ret != 0;
	}

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

	procs_t get_numa_node_id(hwloc_obj_t node, void const* addr) {
		hwloc_nodeset_t ns = hwloc_bitmap_alloc();
		hwloc_membind_policy_t p;
		hwloc_get_area_membind_nodeset(topology, addr, 1, ns, &p, 0);

		int first = hwloc_bitmap_first(ns);
		hwloc_bitmap_free(ns);

		if(first < 0) {
			return std::numeric_limits<procs_t>::max();
		}
		return static_cast<procs_t>(first);
	}
private:
	HWLocSMTTopologyInfo(HWLocSMTTopologyInfo* topo, int depth);

	hwloc_topology_t topology;
	unsigned int root_depth;
	unsigned int numa_depth;
	unsigned int total_depth;
};

template <class Pheet>
HWLocSMTTopologyInfo<Pheet>::HWLocSMTTopologyInfo() {
	/* Allocate and initialize topology object. */
	hwloc_topology_init(&topology);

	/* Perform the topology detection. */
	hwloc_topology_load(topology);

	root_depth = hwloc_get_root_obj(topology)->depth;
	numa_depth = hwloc_get_type_or_above_depth(topology, HWLOC_OBJ_NODE);
	total_depth = hwloc_get_type_or_above_depth(topology, HWLOC_OBJ_PU);
}

template <class Pheet>
HWLocSMTTopologyInfo<Pheet>::~HWLocSMTTopologyInfo() {
	hwloc_topology_destroy(topology);
}

template <class Pheet>
hwloc_obj_t HWLocSMTTopologyInfo<Pheet>::get_root_obj() {
	return hwloc_get_root_obj(topology);
}

template <class Pheet>
unsigned int HWLocSMTTopologyInfo<Pheet>::get_root_depth() {
	return root_depth;
}

template <class Pheet>
unsigned int HWLocSMTTopologyInfo<Pheet>::get_numa_depth() {
	return numa_depth;
}

template <class Pheet>
unsigned int HWLocSMTTopologyInfo<Pheet>::get_total_depth() {
	return total_depth;
}

template <class Pheet>
unsigned int HWLocSMTTopologyInfo<Pheet>::get_total_width() {
	return hwloc_get_nbobjs_by_depth(topology, total_depth);
}

template <class Pheet>
hwloc_cpuset_t HWLocSMTTopologyInfo<Pheet>::get_binding() {
	hwloc_cpuset_t set = hwloc_bitmap_alloc();
	hwloc_get_cpubind(topology, set, HWLOC_CPUBIND_THREAD);
	return set;
}

template <class Pheet>
void HWLocSMTTopologyInfo<Pheet>::bind(hwloc_cpuset_t cpus) {
	hwloc_set_cpubind(topology, cpus, HWLOC_CPUBIND_THREAD);
}

template <class Pheet>
void HWLocSMTTopologyInfo<Pheet>::free_binding(hwloc_cpuset_t cpus) {
	hwloc_bitmap_free(cpus);
}

template <class Pheet>
class HWLocSMTMachineModel {
public:
	typedef HWLocSMTMachineModel<Pheet> ThisType;

	HWLocSMTMachineModel();
	HWLocSMTMachineModel(HWLocSMTMachineModel<Pheet> const& other);
	HWLocSMTMachineModel(HWLocSMTMachineModel<Pheet> const&& other);
	~HWLocSMTMachineModel();

	ThisType& operator=(ThisType const& other);
	ThisType& operator=(ThisType const&& other);

	bool is_leaf();
	procs_t get_num_children();
	HWLocSMTMachineModel<Pheet> get_child(procs_t id);
	procs_t get_node_offset();
	procs_t get_last_leaf_offset();
//	procs_t get_node_id();
	procs_t get_num_leaves();
	procs_t get_memory_level();
	procs_t get_numa_memory_level();
 
	bool supports_SMT() const { return true; }

	void bind();
	void unbind();

	template <typename T>
	bool is_partially_numa_local(T const* addr, size_t count) {
		return topo->is_partially_numa_local(node, addr, count);
	}

	template <typename T>
	bool is_fully_numa_local(T const* addr, size_t count) {
		return topo->is_fully_numa_local(node, addr, count);
	}

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
	HWLocSMTMachineModel(HWLocSMTTopologyInfo<Pheet>* topo, hwloc_obj_t node);
	HWLocSMTTopologyInfo<Pheet>* topo;
	hwloc_obj_t node;
	bool root;

	hwloc_cpuset_t prev_binding;
#ifdef PHEET_DEBUG_MODE
	bool bound;
#endif
};

template <class Pheet>
HWLocSMTMachineModel<Pheet>::HWLocSMTMachineModel()
: topo(new HWLocSMTTopologyInfo<Pheet>()), node(topo->get_root_obj()), root(true), prev_binding(nullptr) {
#ifdef PHEET_DEBUG_MODE
	bound = false;
#endif
}

template <class Pheet>
HWLocSMTMachineModel<Pheet>::HWLocSMTMachineModel(HWLocSMTMachineModel<Pheet> const& other)
: topo(other.topo), node(other.node), root(false), prev_binding(nullptr) {
#ifdef PHEET_DEBUG_MODE
	bound = false;
#endif
}

template <class Pheet>
HWLocSMTMachineModel<Pheet>::HWLocSMTMachineModel(HWLocSMTMachineModel<Pheet> const&& other)
: topo(other.topo), node(other.node), root(false), prev_binding(nullptr) {
#ifdef PHEET_DEBUG_MODE
	bound = false;
#endif
}

template <class Pheet>
HWLocSMTMachineModel<Pheet>::HWLocSMTMachineModel(HWLocSMTTopologyInfo<Pheet>* topo, hwloc_obj_t node)
: topo(topo), node(node), root(false), prev_binding(nullptr) {
#ifdef PHEET_DEBUG_MODE
	bound = false;
#endif
}

template <class Pheet>
HWLocSMTMachineModel<Pheet>::~HWLocSMTMachineModel() {
	if(root) {
		delete topo;
	}
	if(prev_binding != nullptr) {
		topo->free_binding(prev_binding);
	}
}

template <class Pheet>
HWLocSMTMachineModel<Pheet>& HWLocSMTMachineModel<Pheet>::operator=(HWLocSMTMachineModel<Pheet> const& other) {
	pheet_assert(topo == other.topo || !root);
	topo = other.topo;
	node = other.node;
	return *this;
}

template <class Pheet>
HWLocSMTMachineModel<Pheet>& HWLocSMTMachineModel<Pheet>::operator=(HWLocSMTMachineModel<Pheet> const&& other) {
	pheet_assert(topo == other.topo || !root);
	topo = other.topo;
	node = other.node;
	return *this;
}

template <class Pheet>
procs_t HWLocSMTMachineModel<Pheet>::get_num_children() {
	return node->arity;
}

template <class Pheet>
HWLocSMTMachineModel<Pheet> HWLocSMTMachineModel<Pheet>::get_child(procs_t id) {
	pheet_assert(id < node->arity);
	pheet_assert((node->depth) < (topo->get_total_depth()));
	return HWLocSMTMachineModel(topo, node->children[id]);
}

template <class Pheet>
bool HWLocSMTMachineModel<Pheet>::is_leaf() {
	return (node->depth) >= (topo->get_total_depth());
}

template <class Pheet>
void HWLocSMTMachineModel<Pheet>::bind() {
#ifdef PHEET_DEBUG_MODE
	pheet_assert(!bound);
	bound = true;
#endif
	prev_binding = topo->get_binding();
	topo->bind(node->cpuset);
}

template <class Pheet>
void HWLocSMTMachineModel<Pheet>::unbind() {
#ifdef PHEET_DEBUG_MODE
	pheet_assert(bound);
	bound = false;
#endif
	topo->bind(prev_binding);
}

template <class Pheet>
procs_t HWLocSMTMachineModel<Pheet>::get_node_offset() {
	if(!is_leaf()) {
		return get_child(0).get_node_offset();
	}
	return node->logical_index;
}

template <class Pheet>
procs_t HWLocSMTMachineModel<Pheet>::get_last_leaf_offset() {
	if(!is_leaf()) {
		return get_child(get_num_children() - 1).get_last_leaf_offset();
	}
	return node->logical_index;
}
/*
template <class Pheet>
procs_t HWLocSMTMachineModel<Pheet>::get_node_id() {
	return node->logical_index;
}*/

template <class Pheet>
procs_t HWLocSMTMachineModel<Pheet>::get_num_leaves() {
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
procs_t HWLocSMTMachineModel<Pheet>::get_memory_level() {
	return node->depth;
}

template <class Pheet>
procs_t HWLocSMTMachineModel<Pheet>::get_numa_memory_level() {
	return std::min(node->depth, topo->get_numa_depth());
}

}


#endif /* HWLOCSMTMACHINEMODEL_H_ */
