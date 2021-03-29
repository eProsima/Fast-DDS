
// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeSupport.cpp
 */

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <fastcdr/exceptions/Exception.h>


namespace eprosima {
namespace fastdds {
namespace dds {

const InstanceHandle_t HANDLE_NIL;

ReturnCode_t TypeSupport::register_type(
        DomainParticipant* participant,
        std::string type_name) const
{
    return participant->register_type(*this, type_name.empty() ? get_type_name() : type_name);
}

ReturnCode_t TypeSupport::register_type(
        DomainParticipant* participant) const
{
    return participant->register_type(*this, get_type_name());
}

bool TypeSupport::serialize(
        void* data,
        fastrtps::rtps::SerializedPayload_t* payload)
{
    bool result = false;
    try
    {
        result = get()->serialize(data, payload);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        result = false;
    }

    return result;
}

bool TypeSupport::deserialize(
        fastrtps::rtps::SerializedPayload_t* payload,
        void* data)
{
    bool result = false;
    try
    {
        result = get()->deserialize(payload, data);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        result = false;
    }

    return result;
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
