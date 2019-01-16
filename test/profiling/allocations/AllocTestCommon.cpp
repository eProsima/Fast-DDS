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
 * @file AllocTestCommon.cpp
 *
 */

#include "AllocTestCommon.h"

#include <atomic>
#include <iostream>
#include <fstream>
#include <sstream>
#include "osrf_testing_tools_cpp/memory_tools/memory_tools.hpp"

using MemoryToolsService = osrf_testing_tools_cpp::memory_tools::MemoryToolsService;

namespace eprosima_profiling
{

/**
 * Used to run callgrind with --zero-before=*callgrind_zero_count. 
 * See http://valgrind.org/docs/manual/cl-manual.html#cl-manual.options.activity
 */
void callgrind_zero_count() {}

/**
 * Used to run callgrind with --dump-before=*callgrind_dump. 
 * See http://valgrind.org/docs/manual/cl-manual.html#cl-manual.options.activity
 */
void callgrind_dump() {}

static std::atomic_size_t g_allocations[4];
static std::atomic_size_t g_deallocations[4];

static std::atomic_size_t g_phase(0u);
static std::atomic<std::atomic_size_t*> g_allocationsPtr(g_allocations);
static std::atomic<std::atomic_size_t*> g_deallocationsPtr(g_deallocations);

static void allocation_account (MemoryToolsService & service)
{
    (*g_allocationsPtr.load())++;
    service.ignore();
}

static void deallocation_account (MemoryToolsService & service)
{
    (*g_deallocationsPtr.load())++;
    service.ignore();
}

static void next_phase ()
{
    size_t new_phase = ++g_phase;
    if(new_phase < 4)
    {
        g_allocationsPtr.store(&g_allocations[new_phase]);
        g_deallocationsPtr.store(&g_deallocations[new_phase]);
    }
}

/**
 * Called when entities have been created. Memory profiling should begin.
 */
void entities_created() 
{
    // Initialize profiling library
    osrf_testing_tools_cpp::memory_tools::initialize();

    // Set callbacks
    osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc(allocation_account);
    osrf_testing_tools_cpp::memory_tools::on_unexpected_calloc(allocation_account);
    osrf_testing_tools_cpp::memory_tools::on_unexpected_realloc(allocation_account);
    osrf_testing_tools_cpp::memory_tools::on_unexpected_free(deallocation_account);

    // Start profiling
    osrf_testing_tools_cpp::memory_tools::enable_monitoring_in_all_threads();
    EXPECT_NO_MEMORY_OPERATIONS_BEGIN();

    if(!osrf_testing_tools_cpp::memory_tools::is_working())
    {
        std::cerr << "Memory profiler not working!" << std::endl;
    }
}

/**
 * Called after remote entity has been discovered. Data exchange will start.
 */
void discovery_finished() { next_phase(); }

/**
 * Called after first sample has been sent/received.
 */
void first_sample_exchanged() { next_phase(); }

/**
 * Called after all samples have been sent/received. Undiscovery will begin.
 */
void all_samples_exchanged() { next_phase(); }

/**
 * Called after remote entity has been undiscovered. Memory profiling should end.
 */
void undiscovery_finished() 
{ 
    EXPECT_NO_MEMORY_OPERATIONS_END();
}

/**
 * Print memory profiling results.
 */
void print_results(const std::string& file_prefix, const std::string& entity, const std::string& config) 
{
    std::string output_filename = file_prefix;
    if (file_prefix.length() == 0)
    {
        output_filename = "alloc_test_" + entity + "_" + config + ".csv";
    }

    std::ofstream outFile;
    outFile.open(output_filename, std::ofstream::app);

    // Check the file is new
    long pos = outFile.tellp();

    std::stringstream output_stream;

    if(pos == 0)
    {
        output_stream << "\"Phase 0 Allocations\", \"Phase 0 Deallocations\","
            << " \"Phase 1 Allocations\", \"Phase 1 Deallocations\","
            << " \"Phase 2 Allocations\", \"Phase 2 Deallocations\","
            << " \"Phase 3 Allocations\", \"Phase 3 Deallocations\"\n";
    }

    for(size_t i = 0; i < 4; i++)
    {
        size_t allocs = g_allocations[i].load();
        size_t deallocs = g_deallocations[i].load();

        output_stream << allocs << "," << deallocs;
        if (i < 3)
        {
            output_stream << ",";
        }
    }
    output_stream << std::endl;

    outFile << output_stream.str();
    outFile.close();
}

}   // namespace eprosima_profiling

