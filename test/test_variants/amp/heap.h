/*
 * heap.h
 *
 *  Created on: May 03, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef AMP_HEAP_H_
#define AMP_HEAP_H_

namespace pheet {

#define AMP_HEAP_TEST true


// Debug configuration
#define SORTING_TEST true
const procs_t sorting_test_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const unsigned int sorting_test_seeds[] = {0};
const size_t sorting_test_n[] = {5000000};
const int sorting_test_types[] = {0};

/*
// The real thing
#define SORTING_TEST true
const procs_t sorting_test_cpus[] = {1, 2, 3, 6, 12, 24, 48, 96};
const unsigned int sorting_test_seeds[] = {0, 1, 2, 3, 4};
const size_t sorting_test_n[] = {5000000, 50000000};
const int sorting_test_types[] = {0};

*/

}

#endif /* AMP_HEAP_H_ */
