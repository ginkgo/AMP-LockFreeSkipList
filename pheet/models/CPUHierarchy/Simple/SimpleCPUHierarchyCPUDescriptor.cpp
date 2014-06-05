/*
 * SimpleCPUHierarchyCPUDescriptor.cpp
 *
 *  Created on: 13.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#include "SimpleCPUHierarchyCPUDescriptor.h"

namespace pheet {

SimpleCPUHierarchyCPUDescriptor::SimpleCPUHierarchyCPUDescriptor(procs_t id)
: id(id){

}

SimpleCPUHierarchyCPUDescriptor::~SimpleCPUHierarchyCPUDescriptor() {

}

procs_t SimpleCPUHierarchyCPUDescriptor::get_physical_id() {
	return id;
}

}
