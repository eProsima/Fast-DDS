// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file SerializedPayload.cpp
 */

#include <fastdds/rtps/common/SerializedPayload.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

SerializedPayload_t& SerializedPayload_t::operator = (
        SerializedPayload_t&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    encapsulation = other.encapsulation;
    length = other.length;
    data = other.data;
    max_size = other.max_size;
    pos = other.pos;
    payload_owner = other.payload_owner;

    other.encapsulation = CDR_BE;
    other.length = 0;
    other.data = nullptr;
    other.max_size = 0;
    other.pos = 0;
    other.payload_owner = nullptr;

    return *this;
}

SerializedPayload_t::~SerializedPayload_t()
{
    if (payload_owner != nullptr)
    {
        payload_owner->release_payload(*this);
    }
    this->empty();
}

bool SerializedPayload_t::operator == (
        const SerializedPayload_t& other) const
{
    return ((encapsulation == other.encapsulation) &&
           (length == other.length) &&
           (0 == memcmp(data, other.data, length)));
}

bool SerializedPayload_t::copy(
        const SerializedPayload_t* serData,
        bool with_limit)
{
    length = serData->length;

    if (serData->length > max_size)
    {
        if (with_limit)
        {
            return false;
        }
        else
        {
            this->reserve(serData->length);
        }
    }
    encapsulation = serData->encapsulation;
    if (length == 0)
    {
        return true;
    }
    memcpy(data, serData->data, length);
    return true;
}

bool SerializedPayload_t::reserve_fragmented(
        SerializedPayload_t* serData)
{
    length = serData->length;
    max_size = serData->length;
    encapsulation = serData->encapsulation;
    data = (octet*)calloc(length, sizeof(octet));
    return true;
}

void SerializedPayload_t::empty()
{
    assert(payload_owner == nullptr);

    length = 0;
    encapsulation = CDR_BE;
    max_size = 0;
    if (data != nullptr)
    {
        free(data);
    }
    data = nullptr;
}

void SerializedPayload_t::reserve(
        uint32_t new_size)
{
    if (new_size <= this->max_size)
    {
        return;
    }
    if (data == nullptr)
    {
        data = (octet*)calloc(new_size, sizeof(octet));
        if (!data)
        {
            throw std::bad_alloc();
        }
    }
    else
    {
        void* old_data = data;
        data = (octet*)realloc(data, new_size);
        if (!data)
        {
            free(old_data);
            throw std::bad_alloc();
        }
        memset(data + max_size, 0, (new_size - max_size) * sizeof(octet));
    }
    max_size = new_size;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
