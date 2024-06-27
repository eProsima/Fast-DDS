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
 * @file HistoryAttributes.hpp
 *
 */

#ifndef FASTDDS_RTPS_ATTRIBUTES__HISTORYATTRIBUTES_HPP
#define FASTDDS_RTPS_ATTRIBUTES__HISTORYATTRIBUTES_HPP

#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/fastdds_dll.hpp>

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class HistoryAttributes, to specify the attributes of a WriterHistory or a ReaderHistory.
 * This class is only intended to be used with the RTPS API.
 * The Publisher-Subscriber API has other fields to define this values (HistoryQosPolicy and ResourceLimitsQosPolicy).
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class FASTDDS_EXPORTED_API HistoryAttributes
{
public:

    //!Memory management policy.
    MemoryManagementPolicy_t memoryPolicy;

    //!Maximum payload size of the history, default value 500.
    uint32_t payloadMaxSize;

    //!Number of the initial Reserved Caches, default value 500.
    int32_t initialReservedCaches;

    /**
     * Maximum number of reserved caches. Default value is 0 that indicates to keep reserving until something
     * breaks.
     */
    int32_t maximumReservedCaches;

    /**
     * Number of extra caches that can be reserved for other purposes than the history.
     * For example, on a full history, the writer could give as many as these to be used by the application
     * but they will not be able to be inserted in the history unless some cache from the history is released.
     *
     * Default value is 1.
     */
    int32_t extraReservedCaches;

    //! Default constructor
    HistoryAttributes()
        : memoryPolicy(PREALLOCATED_WITH_REALLOC_MEMORY_MODE)
        , payloadMaxSize(500)
        , initialReservedCaches(500)
        , maximumReservedCaches(0)
        , extraReservedCaches(1)
    {
    }

    /** Constructor
     * @param memoryPolicy Set whether memory can be dynamically reallocated or not
     * @param payload Maximum payload size. It is used when memory management policy is
     * PREALLOCATED_MEMORY_MODE or PREALLOCATED_WITH_REALLOC_MEMORY_MODE.
     * @param initial Initial reserved caches. It is used when memory management policy is
     * PREALLOCATED_MEMORY_MODE or PREALLOCATED_WITH_REALLOC_MEMORY_MODE.
     * @param maxRes Maximum reserved caches.
     */
    HistoryAttributes(
            MemoryManagementPolicy_t memoryPolicy,
            uint32_t payload,
            int32_t initial,
            int32_t maxRes)
        : memoryPolicy(memoryPolicy)
        , payloadMaxSize(payload)
        , initialReservedCaches(initial)
        , maximumReservedCaches(maxRes)
        , extraReservedCaches(1)
    {
    }

    /** Constructor
     * @param memoryPolicy Set whether memory can be dynamically reallocated or not
     * @param payload Maximum payload size. It is used when memory management policy is
     * PREALLOCATED_MEMORY_MODE or PREALLOCATED_WITH_REALLOC_MEMORY_MODE.
     * @param initial Initial reserved caches. It is used when memory management policy is
     * PREALLOCATED_MEMORY_MODE or PREALLOCATED_WITH_REALLOC_MEMORY_MODE.
     * @param maxRes Maximum reserved caches.
     * @param extra Extra reserved caches.
     */
    HistoryAttributes(
            MemoryManagementPolicy_t memoryPolicy,
            uint32_t payload,
            int32_t initial,
            int32_t maxRes,
            int32_t extra)
        : memoryPolicy(memoryPolicy)
        , payloadMaxSize(payload)
        , initialReservedCaches(initial)
        , maximumReservedCaches(maxRes)
        , extraReservedCaches(extra)
    {
    }

    virtual ~HistoryAttributes()
    {
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_ATTRIBUTES__HISTORYATTRIBUTES_HPP
