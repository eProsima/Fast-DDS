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
 * @file Preallocated.hpp
 */

template <>
class Impl<PREALLOCATED_MEMORY_MODE> : public BaseImpl
{
public:

    explicit Impl(
            uint32_t payload_size)
        : payload_size_(payload_size)
    {
    }

    bool get_payload(
            uint32_t /* size */,
            CacheChange_t& cache_change) override
    {
        cache_change.serializedPayload.reserve(payload_size_);
        cache_change.payload_owner(this);
        return true;
    }

    bool get_payload(
            SerializedPayload_t& data,
            IPayloadPool*& /* data_owner */,
            CacheChange_t& cache_change) override
    {
        assert(cache_change.writer_guid != GUID_t::unknown());
        assert(cache_change.sequence_number != SequenceNumber_t::unknown());

        cache_change.serializedPayload.reserve(payload_size_);
        if (cache_change.serializedPayload.copy(&data, true))
        {
            cache_change.payload_owner(this);
            return true;
        }

        return false;
    }

private:

    uint32_t payload_size_;
};
