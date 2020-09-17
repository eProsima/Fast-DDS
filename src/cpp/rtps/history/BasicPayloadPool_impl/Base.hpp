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
 * @file Base.hpp
 */

class BaseImpl : public IPayloadPool
{
    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        cache_change.serializedPayload.reserve(size);
        return true;
    }

    bool get_payload(
            const SerializedPayload_t& data,
            const IPayloadPool* /* data_owner */,
            CacheChange_t& cache_change) override
    {
        return cache_change.serializedPayload.copy(&data, false);
    }

    bool release_payload(
            CacheChange_t& cache_change) override
    {
        cache_change.serializedPayload.length = 0;
        cache_change.serializedPayload.pos = 0;

        return true;
    }

};

template <
    class SizeGrowCalculator,
    MemoryManagementPolicy_t policy_>
class Impl : public BaseImpl
{
};

