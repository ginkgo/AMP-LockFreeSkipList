/*
 * SimpleCPUHierarchy.h
 *
 *  Created on: 13.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SIMPLECPUHIERARCHY_H_
#define SIMPLECPUHIERARCHY_H_

#include "../../../settings.h"
#include "../../../misc/types.h"

#include "SimpleCPUHierarchyCPUDescriptor.h"

#include <vector>

namespace pheet {


class SimpleCPUHierarchy {
public:
	typedef SimpleCPUHierarchyCPUDescriptor CPUDescriptor;

	SimpleCPUHierarchy(procs_t np);
	SimpleCPUHierarchy(procs_t np, procs_t memory_level);
	SimpleCPUHierarchy(procs_t* levels, procs_t num_levels);
	SimpleCPUHierarchy(procs_t* levels, procs_t num_levels, procs_t memory_level);
	~SimpleCPUHierarchy();

	procs_t get_size();
	std::vector<SimpleCPUHierarchy*> const* get_subsets();
	std::vector<CPUDescriptor*> const* get_cpus();
	procs_t get_max_depth();
	procs_t get_memory_level();

private:
	SimpleCPUHierarchy(SimpleCPUHierarchy* parent, procs_t offset);
	void autogen_levels(procs_t np);

	procs_t level;
	procs_t num_levels;
	procs_t offset;
	procs_t memory_level;
	procs_t* levels;
	std::vector<SimpleCPUHierarchy*> subsets;
	std::vector<CPUDescriptor*> cpus;
};

}

#endif /* SIMPLECPUHIERARCHY_H_ */
