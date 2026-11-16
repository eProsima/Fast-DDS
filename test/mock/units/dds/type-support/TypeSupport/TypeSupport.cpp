// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of TypeSupport: replaces src/cpp/fastdds/topic/TypeSupport.cpp. Registering a type forwards
// to the TypeSupportMock instance, if any, instead of the DomainParticipant.

#include <fastcdr/exceptions/Exception.h>

#include <fastdds/dds/topic/TypeSupport.hpp>

#include <mock/fastdds/dds/topic/TypeSupportMock.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

const InstanceHandle_t HANDLE_NIL;

TypeSupportMock*& TypeSupportMock::instance()
{
    static TypeSupportMock* instance = nullptr;
    return instance;
}

ReturnCode_t TypeSupport::register_type(
        DomainParticipant* participant,
        std::string type_name) const
{
    if (nullptr != TypeSupportMock::instance())
    {
        return TypeSupportMock::instance()->register_type(participant,
                       type_name.empty() ? get_type_name() : type_name);
    }
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t TypeSupport::register_type(
        DomainParticipant* participant) const
{
    return register_type(participant, std::string());
}

bool TypeSupport::serialize(
        const void* const data,
        fastdds::rtps::SerializedPayload_t& payload,
        DataRepresentationId_t data_representation)
{
    try
    {
        return get()->serialize(data, payload, data_representation);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        return false;
    }
}

bool TypeSupport::deserialize(
        fastdds::rtps::SerializedPayload_t& payload,
        void* data)
{
    try
    {
        return get()->deserialize(payload, data);
    }
    catch (eprosima::fastcdr::exception::Exception&)
    {
        return false;
    }
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
