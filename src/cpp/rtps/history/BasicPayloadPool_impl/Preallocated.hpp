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
        assert(payload_size_ > 0);
    }

    bool get_payload(
            uint32_t /* size */,
            SerializedPayload_t& payload) override
    {
        payload.reserve(payload_size_);
        payload.payload_owner = this;
        return true;
    }

    bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) override
    {
        payload.reserve(payload_size_);
        if (payload.copy(&data, true))
        {
            payload.payload_owner = this;
            return true;
        }

        return false;
    }

private:

    uint32_t payload_size_;
};
