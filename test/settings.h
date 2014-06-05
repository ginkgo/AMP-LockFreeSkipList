/*
 * settings.h
 *
 *  Created on: Feb 15, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PHEET_TEST_SETTINGS_H_
#define PHEET_TEST_SETTINGS_H_

//#define PHEET_DEBUG_MODE
// #define PHEET_ALL_PERFORMANCE_COUNTERS
#define ACTIVE_TEST 	"test_variants/ds.h"

bool const graph_bipartitioning_test_count_irrelevant_tasks = false;
bool const graph_bipartitioning_test_count_upper_bound_changes = false;
bool const graph_bipartitioning_test_count_allocated_subproblems = false;
bool const graph_bipartitioning_test_measure_memory_allocation_time = false;
bool const graph_bipartitioning_test_measure_last_update_time = true;

bool const lupiv_track_locality = false;
bool const lupiv_count_locality_misses = false;
bool const lupiv_count_total_distance = false;
//bool const lupiv_slices_rescheduled_at_same_place = true;
//bool const lupiv_total_distance_to_last_location = true;
bool const lupiv_total_tasks = false;

bool const sor_events = false;
bool const sor_average_distance = false;
bool const sor_slices_rescheduled_at_same_place = false;

bool const prefix_sum_log_pf_blocks = false;
bool const prefix_sum_log_pf_preprocessed_blocks = false;
bool const prefix_sum_log_numa_local_blocks = false;
bool const prefix_sum_log_numa_non_local_blocks = false;
bool const prefix_sum_log_numa_cache_time = false;
bool const prefix_sum_log_schedule = false;

bool const sssp_count_dead_tasks = false;
bool const sssp_count_actual_tasks = false;
bool const sssp_measure_last_non_dead_time = false;
bool const sssp_measure_last_task_time = false;
bool const sssp_measure_last_update_time = false;

#endif /* PHEET_TEST_SETTINGS_H_ */
