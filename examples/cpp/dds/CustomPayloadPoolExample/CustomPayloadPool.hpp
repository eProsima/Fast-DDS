// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file CustomPayloadPool.hpp
 */

#ifndef DDS_CUSTOM_PAYLOAD_POOL_DATA_HPP
#define DDS_CUSTOM_PAYLOAD_POOL_DATA_HPP

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/common/CacheChange.h>

class CustomPayloadPool : public eprosima::fastrtps::rtps::IPayloadPool
{
public:

    ~CustomPayloadPool() = default;

    bool get_payload(
            unsigned int size,
            eprosima::fastrtps::rtps::CacheChange_t& cache_change)
    {
        // Reserve new memory for the payload buffer
        unsigned char* payload = new unsigned char[size];

        // Assign the payload buffer to the CacheChange and update sizes
        cache_change.serializedPayload.data = payload;
        cache_change.serializedPayload.length = size;
        cache_change.serializedPayload.max_size = size;

        // Tell the CacheChange who needs to release its payload
        cache_change.payload_owner(this);

        return true;
    }

    bool get_payload(
            eprosima::fastrtps::rtps::SerializedPayload_t& data,
            eprosima::fastrtps::rtps::IPayloadPool*& /*data_owner*/,
            eprosima::fastrtps::rtps::CacheChange_t& cache_change)
    {
        // Reserve new memory for the payload buffer
        unsigned char* payload = new unsigned char[data.length];

        // Copy the data
        memcpy(payload, data.data, data.length);

        // Tell the CacheChange who needs to release its payload
        cache_change.payload_owner(this);

        // Assign the payload buffer to the CacheChange and update sizes
        cache_change.serializedPayload.data = payload;
        cache_change.serializedPayload.length = data.length;
        cache_change.serializedPayload.max_size = data.length;

        return true;
    }

    bool release_payload(
            eprosima::fastrtps::rtps::CacheChange_t& cache_change)
    {
        // Ensure precondition
        if (this != cache_change.payload_owner())
        {
            std::cerr << "Trying to release a payload buffer allocated by a different PayloadPool." << std::endl;
            return false;
        }

        // Dealloc the buffer of the payload
        delete[] cache_change.serializedPayload.data;

        // Reset sizes and pointers
        cache_change.serializedPayload.data = nullptr;
        cache_change.serializedPayload.length = 0;
        cache_change.serializedPayload.max_size = 0;

        // Reset the owner of the payload
        cache_change.payload_owner(nullptr);

        return true;
    }

};

#endif  // DDS_CUSTOM_PAYLOAD_POOL_DATA_HPP
