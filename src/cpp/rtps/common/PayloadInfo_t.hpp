// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PayloadInfo_t.hpp
 */

#ifndef RTPS_COMMON_PAYLOADINFO_T_HPP_
#define RTPS_COMMON_PAYLOADINFO_T_HPP_

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/rtps/history/IPayloadPool.h>

#include <cassert>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace detail {

struct PayloadInfo_t
{
    SerializedPayload_t payload;
    IPayloadPool* payload_owner = nullptr;

    ~PayloadInfo_t()
    {
        // Avoid payload.data to be freed
        payload.data = nullptr;
        payload.length = 0;
    }

    void move_from_change(
            CacheChange_t& change)
    {
        // This class must be empty
        assert(payload_owner == nullptr);
        assert(payload.data == nullptr);
        assert(payload.length == 0);

        payload_owner = change.payload_owner();
        change.payload_owner(nullptr);

        payload = change.serializedPayload;
        change.serializedPayload.data = nullptr;
        change.serializedPayload.length = 0;
        change.serializedPayload.pos = 0;
        change.serializedPayload.max_size = 0;
    }

    void move_into_change(
            CacheChange_t& change)
    {
        // Output change must be empty
        assert(change.payload_owner() == nullptr);
        assert(change.serializedPayload.data == nullptr);
        assert(change.serializedPayload.length == 0);

        change.payload_owner(payload_owner);
        payload_owner = nullptr;

        change.serializedPayload = payload;
        payload.data = nullptr;
        payload.length = 0;
        payload.pos = 0;
        payload.max_size = 0;
    }

    void copy_reference_from_change(
            const CacheChange_t& change)
    {
        // This class must be empty
        assert(payload_owner == nullptr);
        assert(payload.data == nullptr);
        assert(payload.length == 0);

        // Input change must have content
        assert(change.payload_owner() != nullptr);
        assert(change.serializedPayload.data != nullptr);
        assert(change.serializedPayload.length > 0);

        // Let the payload pool of the input change copy the payload.
        // This will usually just increment the reference counter of the payload.
        IPayloadPool* input_pool = const_cast<IPayloadPool*>(change.payload_owner());
        SerializedPayload_t input_payload = change.serializedPayload;
        CacheChange_t change_copy;
        change_copy.copy_not_memcpy(&change);
        input_pool->get_payload(input_payload, input_pool, change_copy);

        // Pool and payload must not have been modified
        assert(change.payload_owner() == input_pool);
        assert(change.serializedPayload.data == input_payload.data);
        assert(change.serializedPayload.length == input_payload.length);

        // Reset the local variable, so the pointer is not free'd when going out of scope
        input_payload.data = nullptr;
        input_payload.length = 0;

        // Keep the payload and owner
        move_from_change(change_copy);
    }
};

}  // namespace detail
}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_COMMON_PAYLOADINFO_T_HPP_