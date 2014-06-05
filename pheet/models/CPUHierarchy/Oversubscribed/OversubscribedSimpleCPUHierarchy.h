/*
 * OversubscribedSimpleCPUHierarchy.h
 *
 *  Created on: 20.04.2011
 *      Author: Martin Wimmer
 */

#ifndef OVERSUBSCRIBEDSIMPLECPUHIERARCHY_H_
#define OVERSUBSCRIBEDSIMPLECPUHIERARCHY_H_

#include "../Simple/SimpleCPUHierarchy.h"

namespace pheet {

class OversubscribedSimpleCPUHierarchy {
public:
	typedef SimpleCPUHierarchy::CPUDescriptor CPUDescriptor;

	OversubscribedSimpleCPUHierarchy(procs_t np);
	~OversubscribedSimpleCPUHierarchy();

	procs_t get_size();
	std::vector<OversubscribedSimpleCPUHierarchy*> const* get_subsets();
	std::vector<CPUDescriptor*> const* get_cpus();
	procs_t get_max_depth();
	procs_t get_memory_level();

private:
	OversubscribedSimpleCPUHierarchy(SimpleCPUHierarchy* simple_hierarchy);

	procs_t np;
	procs_t memory_levels;
	SimpleCPUHierarchy* simple_hierarchy;
	bool allocated_simple_hierarchy;

	std::vector<OversubscribedSimpleCPUHierarchy*> subsets;
	std::vector<CPUDescriptor*> cpus;
};

}

#endif /* OVERSUBSCRIBEDSIMPLECPUHIERARCHY_H_ */
