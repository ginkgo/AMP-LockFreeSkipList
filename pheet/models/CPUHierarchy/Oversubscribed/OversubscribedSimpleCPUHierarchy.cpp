/*
 * OversubscribedSimpleCPUHierarchy.cpp
 *
 *  Created on: 20.04.2011
 *      Author: Martin Wimmer
 */

#include "OversubscribedSimpleCPUHierarchy.h"

namespace pheet {

OversubscribedSimpleCPUHierarchy::OversubscribedSimpleCPUHierarchy(procs_t np)
: np(np), memory_levels(0), simple_hierarchy(NULL), allocated_simple_hierarchy(false) {

}

OversubscribedSimpleCPUHierarchy::OversubscribedSimpleCPUHierarchy(SimpleCPUHierarchy* simple_hierarchy)
: np(simple_hierarchy->get_size()), memory_levels(1), simple_hierarchy(simple_hierarchy), allocated_simple_hierarchy(false) {

}

OversubscribedSimpleCPUHierarchy::~OversubscribedSimpleCPUHierarchy() {
	for(size_t i = 0; i < subsets.size(); i++) {
		delete subsets[i];
	}
	for(size_t i = 0; i < cpus.size(); i++) {
		delete cpus[i];
	}
	if(allocated_simple_hierarchy) {
		delete simple_hierarchy;
	}
}

procs_t OversubscribedSimpleCPUHierarchy::get_size() {
	return np;
}

std::vector<OversubscribedSimpleCPUHierarchy*> const* OversubscribedSimpleCPUHierarchy::get_subsets() {
	if(subsets.size() == 0) {
		if(np <= system_max_cpus) {
			if(simple_hierarchy == NULL) {
				simple_hierarchy = new SimpleCPUHierarchy(np, 1);
				allocated_simple_hierarchy = true;
			}
			std::vector<SimpleCPUHierarchy*> const* simple_sub = simple_hierarchy->get_subsets();
			for(size_t i = 0; i < simple_sub->size(); i++) {
				subsets.push_back(new OversubscribedSimpleCPUHierarchy((*simple_sub)[i]));
			}
		}
		else {
			procs_t i;
			for ( i = np; i > system_max_cpus; i -= system_max_cpus) {
				subsets.push_back(new OversubscribedSimpleCPUHierarchy(system_max_cpus));
			}
			subsets.push_back(new OversubscribedSimpleCPUHierarchy(i));
		}
	}
	return &subsets;
}

std::vector<OversubscribedSimpleCPUHierarchy::CPUDescriptor*> const* OversubscribedSimpleCPUHierarchy::get_cpus() {
	if(np <= system_max_cpus) {
		if(simple_hierarchy == NULL) {
			simple_hierarchy = new SimpleCPUHierarchy(np, 1);
			allocated_simple_hierarchy = true;
		}
		return simple_hierarchy->get_cpus();
	}
	if(cpus.size() == 0) {
		cpus.reserve(get_size());

		for(procs_t i = 0; i < system_max_cpus; i++) {
			SimpleCPUHierarchy::CPUDescriptor* desc = new SimpleCPUHierarchy::CPUDescriptor(i);
			cpus.push_back(desc);
		}
	}
	return &cpus;
}

procs_t OversubscribedSimpleCPUHierarchy::get_max_depth() {
	if(np <= system_max_cpus) {
		return 1;
	}
	std::vector<OversubscribedSimpleCPUHierarchy*> const* subsets = get_subsets();
	procs_t depth = 0;
	for(size_t i = 0; i < subsets->size(); ++i) {
		procs_t tmp = (*subsets)[i]->get_max_depth();
		if(tmp > depth) {
			depth = tmp;
		}
	}
	return depth + 1;
}

procs_t OversubscribedSimpleCPUHierarchy::get_memory_level() {
	if(np <= system_max_cpus) {
		if(simple_hierarchy == NULL) {
			simple_hierarchy = new SimpleCPUHierarchy(np, 1);
			allocated_simple_hierarchy = true;
		}
		return simple_hierarchy->get_memory_level();
	}
	else {
		return 0;
	}
}

}
