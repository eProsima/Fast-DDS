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
class Impl<PREALLOCATED_WITH_REALLOC_MEMORY_MODE> : public BaseImpl
{
public:

    explicit Impl(
            uint32_t payload_size)
        : min_payload_size_(payload_size)
    {
        assert(min_payload_size_ > 0);
    }

    bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) override
    {
        payload.reserve(std::max(size, min_payload_size_));
        payload.payload_owner = this;
        return true;
    }

private:

    using BaseImpl::get_payload;

    uint32_t min_payload_size_;
};
