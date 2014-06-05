/*
 * SimpleCPUHierarchy.cpp
 *
 *  Created on: 13.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#include "SimpleCPUHierarchy.h"

#include <cstring>

namespace pheet {

SimpleCPUHierarchy::SimpleCPUHierarchy(procs_t np)
: level(0), offset(0), memory_level(0) {
	autogen_levels(np);
}

SimpleCPUHierarchy::SimpleCPUHierarchy(procs_t np, procs_t memory_level)
: level(0), offset(0), memory_level(memory_level) {
	autogen_levels(np);
}

SimpleCPUHierarchy::SimpleCPUHierarchy(procs_t* levels, procs_t num_levels)
: level(0), num_levels(num_levels), offset(0), memory_level(0) {
	pheet_assert(num_levels >= 1);
	this->levels = new procs_t[num_levels];
	memcpy(this->levels, levels, sizeof(procs_t) * num_levels);
}

SimpleCPUHierarchy::SimpleCPUHierarchy(procs_t* levels, procs_t num_levels, procs_t memory_level)
: level(0), num_levels(num_levels), offset(0), memory_level(memory_level) {
	pheet_assert(num_levels >= 1);
	this->levels = new procs_t[num_levels];
	memcpy(this->levels, levels, sizeof(procs_t) * num_levels);
}

SimpleCPUHierarchy::SimpleCPUHierarchy(SimpleCPUHierarchy* parent, procs_t offset)
: level(parent->level + 1), num_levels(parent->num_levels - 1), offset(offset), memory_level(parent->memory_level + 1), levels(parent->levels + 1) {

}

SimpleCPUHierarchy::~SimpleCPUHierarchy() {
	if(level == 0) {
		delete []levels;
	}
	for(size_t i = 0; i < subsets.size(); i++) {
		delete subsets[i];
	}
	for(size_t i = 0; i < cpus.size(); i++) {
		delete cpus[i];
	}
}

void SimpleCPUHierarchy::autogen_levels(procs_t np) {
	size_t num_sys_hier_levels = sizeof(system_cpu_hierarchy)/sizeof(system_cpu_hierarchy[0]);

	while(num_sys_hier_levels > 1 && system_cpu_hierarchy[num_sys_hier_levels-2] >= np) {
		--num_sys_hier_levels;
	}

	// Generates a flat hierarchy
	num_levels = num_sys_hier_levels + 1;
	levels = new procs_t[num_levels];
	levels[0] = np;
	for(procs_t i = 1; i < num_sys_hier_levels; ++i) {
		levels[i] = system_cpu_hierarchy[num_sys_hier_levels - i - 1];
	}
	levels[num_sys_hier_levels] = 1;
}

procs_t SimpleCPUHierarchy::get_size() {
	return levels[0];
}

std::vector<SimpleCPUHierarchy*> const* SimpleCPUHierarchy::get_subsets() {
	if(num_levels > 1 && subsets.size() == 0) {
		pheet_assert((levels[0] % levels[1]) == 0);
		procs_t groups = levels[0] / levels[1];
		subsets.reserve(groups);

		for(procs_t i = 0; i < groups; i++) {
			SimpleCPUHierarchy* sub = new SimpleCPUHierarchy(this, offset + i*levels[1]);
			subsets.push_back(sub);
		}
	}
	return &subsets;
}

std::vector<SimpleCPUHierarchy::CPUDescriptor*> const* SimpleCPUHierarchy::get_cpus() {
	if(cpus.size() == 0) {
		cpus.reserve(get_size());

		for(procs_t i = 0; i < get_size(); i++) {
			SimpleCPUHierarchy::CPUDescriptor* desc = new SimpleCPUHierarchy::CPUDescriptor(i + offset);
			cpus.push_back(desc);
		}
	}
	return &cpus;
}

procs_t SimpleCPUHierarchy::get_max_depth() {
	return 1;
}

procs_t SimpleCPUHierarchy::get_memory_level() {
	return memory_level;
}

}
