/*
 * types.h
 *
 *  Created on: 06.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stddef.h>
#include <stdint.h>

namespace pheet {

typedef size_t procs_t;
typedef ptrdiff_t s_procs_t;
#define PROCST_FETCH_AND_SUB(p, val)		(SIZET_FETCH_AND_SUB(p, val))

}

#endif /* TYPES_H_ */
