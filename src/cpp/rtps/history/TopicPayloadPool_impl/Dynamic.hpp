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
 * @file Dynamic.hpp
 */

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace TopicPayloadPool {

template <>
class Impl<DYNAMIC_RESERVE_MEMORY_MODE> : public BaseImpl
{
public:

    virtual bool release_payload(
            CacheChange_t& cache_change) override
    {
        assert(cache_change.payload_owner() == this);

        // Find data in allPayloads, remove element, then delete it
        std::vector<octet*>::iterator target =
                std::find(all_payloads_.begin(), all_payloads_.end(), cache_change.serializedPayload.data);
        if (target != all_payloads_.end())
        {
            // Copy last element into the element being removed
            if (target != --all_payloads_.end())
            {
                *target = all_payloads_.back();
            }

            // Then drop last element
            all_payloads_.pop_back();
        }
        else
        {
            logError(RTPS_HISTORY, "Trying to release a payload that is not logged in the Pool");
            return false;
        }

        // Now we can free the memory
        free(cache_change.serializedPayload.data);

        cache_change.serializedPayload.length = 0;
        cache_change.serializedPayload.pos = 0;
        cache_change.serializedPayload.max_size = 0;
        cache_change.serializedPayload.data = nullptr;
        cache_change.payload_owner(nullptr);

        return true;
    }

protected:

    MemoryManagementPolicy_t memory_policy() const
    {
        return DYNAMIC_RESERVE_MEMORY_MODE;
    }

};

}  // namespace TopicPayloadPool
}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
