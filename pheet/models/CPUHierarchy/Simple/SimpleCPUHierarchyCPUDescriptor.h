/*
 * SimpleCPUHierarchyCPUDescriptor.h
 *
 *  Created on: 13.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SIMPLECPUHIERARCHYCPUDESCRIPTOR_H_
#define SIMPLECPUHIERARCHYCPUDESCRIPTOR_H_

#include "../../../misc/types.h"

namespace pheet {

class SimpleCPUHierarchyCPUDescriptor {
public:
	SimpleCPUHierarchyCPUDescriptor(procs_t id);
	~SimpleCPUHierarchyCPUDescriptor();

	procs_t get_physical_id();
private:
	procs_t id;
};

}

#endif /* SIMPLECPUHIERARCHYCPUDESCRIPTOR_H_ */
