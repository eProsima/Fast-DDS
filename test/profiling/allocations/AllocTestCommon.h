// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file AllocTestCommon.h
 *
 */

#ifndef FASTDDS_TEST_PROFILING_ALLOCATIONS_ALLOCTESTCOMMON_H_
#define FASTDDS_TEST_PROFILING_ALLOCATIONS_ALLOCTESTCOMMON_H_

#include <string>

namespace eprosima_profiling {

/**
 * Used to run callgrind with --zero-before=callgrind_zero_count.
 * See http://valgrind.org/docs/manual/cl-manual.html#cl-manual.options.activity
 */
void callgrind_zero_count();

/**
 * Used to run callgrind with --dump-before=callgrind_dump.
 * See http://valgrind.org/docs/manual/cl-manual.html#cl-manual.options.activity
 */
void callgrind_dump();

/**
 * Called when entities have been created. Memory profiling should begin.
 *
 * @param print_alloc_traces    Indicates whether to print backtrace for allocations
 * @param print_dealloc_traces  Indicates whether to print backtrace for deallocations
 */
void entities_created(
        bool print_alloc_traces = false,
        bool print_dealloc_traces = false);

/**
 * Called after remote entity has been discovered. Data exchange will start.
 */
void discovery_finished();

/**
 * Called after first sample has been sent/received.
 */
void first_sample_exchanged();

/**
 * Called after all samples have been sent/received. Undiscovery will begin.
 */
void all_samples_exchanged();

/**
 * Called after remote entity has been undiscovered. Memory profiling should end.
 */
void undiscovery_finished();

/**
 * Print memory profiling results.
 */
void print_results(
        const std::string& file_prefix,
        const std::string& entity,
        const std::string& config);

}   // namespace eprosima_profiling

#endif   // FASTDDS_TEST_PROFILING_ALLOCATIONS_ALLOCTESTCOMMON_H_
