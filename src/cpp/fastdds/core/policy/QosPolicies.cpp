// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ParameterTypes.cpp
 *
 */

#include <fastdds/dds/core/policy/QosPolicies.hpp>

#include <fastdds/rtps/messages/CDRMessage.h>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace eprosima::fastrtps::rtps;

uint32_t QosPolicy::get_cdr_serialized_size(
        const std::vector<fastrtps::rtps::octet>& data)
{
    // Size of data
    uint32_t data_size = static_cast<uint32_t>(data.size());
    // Align to next 4 byte
    data_size = (data_size + 3) & ~3;
    // p_id + p_length + str_length + str_data
    return 2 + 2 + 4 + data_size;
}

uint32_t PartitionQosPolicy::cdr_serialized_size() const
{
    // p_id + p_length + partition_number
    uint32_t ret_val = 2 + 2 + 4;
    for (PartitionQosPolicy::const_iterator it = this->begin(); it != this->end(); ++it)
    {
        // str_size
        ret_val += 4;
        // str_data (including null char)
        ret_val += static_cast<uint32_t>(it->size());
        // align
        ret_val = (ret_val + 3) & ~3;
    }

    return ret_val;
}

uint32_t DataRepresentationQosPolicy::cdr_serialized_size() const
{
    // Size of data
    uint32_t data_size = static_cast<uint32_t>(m_value.size() * sizeof(uint16_t));
    // Align to next 4 byte
    data_size = (data_size + 3) & ~3;
    // p_id + p_length + data_size + data
    return 2 + 2 + 4 + data_size;
}

uint32_t TypeIdV1::cdr_serialized_size() const
{
    size_t size = fastrtps::types::TypeIdentifier::getCdrSerializedSize(m_type_identifier) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

uint32_t TypeObjectV1::cdr_serialized_size() const
{
    size_t size = fastrtps::types::TypeObject::getCdrSerializedSize(m_type_object) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

uint32_t xtypes::TypeInformation::cdr_serialized_size() const
{
    size_t size = fastrtps::types::TypeInformation::getCdrSerializedSize(type_information) + 4;
    return 2 + 2 + static_cast<uint32_t>(size);
}

} //namespace dds
} //namespace fastdds
} //namespace eprosima
