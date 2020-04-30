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
 * @file ParameterSerializer.hpp
 *
 */

#ifndef FASTDDS_CORE_POLICY__PARAMETERSERIALIZER_HPP_
#define FASTDDS_CORE_POLICY__PARAMETERSERIALIZER_HPP_

#include "ParameterList.hpp"
#include <fastdds/rtps/common/CDRMessage_t.h>

namespace eprosima {
namespace fastdds {
namespace dds {

template <typename Parameter>
class ParameterSerializer
{
public:

    static inline bool add_common_to_cdr_message(
            const Parameter& parameter,
            fastrtps::rtps::CDRMessage_t* cdr_message)
    {
        bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, parameter.Pid);
        valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, parameter.length);
        return valid;
    }

    static bool add_to_cdr_message(
            const Parameter& parameter,
            fastrtps::rtps::CDRMessage_t* cdr_message)
    {
        bool valid = add_common_to_cdr_message(parameter, cdr_message);
        valid &= add_content_to_cdr_message(parameter, cdr_message);
        return valid;
    }

    static bool read_from_cdr_message(
            Parameter& parameter,
            fastrtps::rtps::CDRMessage_t* cdr_message,
            const uint16_t parameter_length)
    {
        bool valid = true;
        valid &= read_content_from_cdr_message(parameter, cdr_message, parameter_length);
        return valid;
    }

    static uint32_t cdr_serialized_size(
            const Parameter& parameter)
    {
        return 4 + parameter.length;
    }

private:

    static bool add_content_to_cdr_message(
            const Parameter&,
            fastrtps::rtps::CDRMessage_t*)
    {
        static_assert(sizeof(Parameter) == 0, "Not implemented");
        return false;
    }

    static bool read_content_from_cdr_message(
            Parameter&,
            fastrtps::rtps::CDRMessage_t*,
            const uint16_t)
    {
        static_assert(sizeof(Parameter) == 0, "Not implemented");
        return false;
    }

};

template<>
class ParameterSerializer<Parameter_t>
{
public:

    static bool add_parameter_status(
            fastrtps::rtps::CDRMessage_t* cdr_message,
            fastrtps::rtps::octet status)
    {
        if (cdr_message->pos + 8 >= cdr_message->max_size)
        {
            return false;
        }
        fastrtps::rtps::CDRMessage::addUInt16(cdr_message, fastdds::dds::PID_STATUS_INFO);
        fastrtps::rtps::CDRMessage::addUInt16(cdr_message, 4);
        fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
        fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
        fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
        fastrtps::rtps::CDRMessage::addOctet(cdr_message, status);
        return true;
    }

    static bool add_parameter_key(
            fastrtps::rtps::CDRMessage_t* cdr_message,
            const fastrtps::rtps::InstanceHandle_t& iHandle)
    {
        if (cdr_message->pos + 20 >= cdr_message->max_size)
        {
            return false;
        }
        fastrtps::rtps::CDRMessage::addUInt16(cdr_message, fastdds::dds::PID_KEY_HASH);
        fastrtps::rtps::CDRMessage::addUInt16(cdr_message, 16);
        fastrtps::rtps::CDRMessage::addData(cdr_message, iHandle.value, 16);
        return true;
    }

    static bool add_parameter_sentinel(
            fastrtps::rtps::CDRMessage_t* cdr_message)
    {
        if (cdr_message->pos + 4 > cdr_message->max_size)
        {
            return false;
        }
        fastrtps::rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(fastdds::dds::PID_SENTINEL));
        fastrtps::rtps::CDRMessage::addUInt16(cdr_message, 0);

        return true;
    }

    static bool add_parameter_sample_identity(
            fastrtps::rtps::CDRMessage_t* cdr_message,
            const fastrtps::rtps::SampleIdentity& sample_id)
    {
        if (cdr_message->pos + 28 > cdr_message->max_size)
        {
            return false;
        }

        fastrtps::rtps::CDRMessage::addUInt16(cdr_message, fastdds::dds::PID_RELATED_SAMPLE_IDENTITY);
        fastrtps::rtps::CDRMessage::addUInt16(cdr_message, 24);
        fastrtps::rtps::CDRMessage::addData(cdr_message,
                sample_id.writer_guid().guidPrefix.value, fastrtps::rtps::GuidPrefix_t::size);
        fastrtps::rtps::CDRMessage::addData(cdr_message,
                sample_id.writer_guid().entityId.value, fastrtps::rtps::EntityId_t::size);
        fastrtps::rtps::CDRMessage::addInt32(cdr_message, sample_id.sequence_number().high);
        fastrtps::rtps::CDRMessage::addUInt32(cdr_message, sample_id.sequence_number().low);
        return true;
    }

    static inline uint32_t cdr_serialized_size(
            const fastrtps::string_255& str)
    {
        // Size including NUL char at the end
        uint32_t str_siz = static_cast<uint32_t>(str.size()) + 1;
        // Align to next 4 byte
        str_siz = (str_siz + 3) & ~3;
        // p_id + p_length + str_length + str_data
        return 2 + 2 + 4 + str_siz;
    }

    static inline uint32_t cdr_serialized_size(
            const fastrtps::rtps::Token& token)
    {
        // p_id + p_length
        uint32_t ret_val = 2 + 2;

        // str_len + null_char + str_data
        ret_val += 4 + 1 + static_cast<uint32_t>(strlen(token.class_id().c_str()));
        // align
        ret_val = (ret_val + 3) & ~3;

        // properties
        ret_val += static_cast<uint32_t>(fastrtps::rtps::PropertyHelper::serialized_size(token.properties()));
        // align
        ret_val = (ret_val + 3) & ~3;

        // binary_properties
        ret_val +=
                static_cast<uint32_t>(fastrtps::rtps::BinaryPropertyHelper::serialized_size(
                    token.binary_properties()));
        // align
        ret_val = (ret_val + 3) & ~3;

        return ret_val;
    }

};

template<>
inline bool ParameterSerializer<ParameterLocator_t>::add_content_to_cdr_message(
        const ParameterLocator_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return fastrtps::rtps::CDRMessage::addLocator(cdr_message, parameter.locator);
}

template<>
inline bool ParameterSerializer<ParameterLocator_t>::read_content_from_cdr_message(
        ParameterLocator_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_LOCATOR_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readLocator(cdr_message, &parameter.locator);
}

template<>
inline bool ParameterSerializer<ParameterKey_t>::add_to_cdr_message(
        const ParameterKey_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return ParameterSerializer<Parameter_t>::add_parameter_key(cdr_message, parameter.key);
}

template<>
inline bool ParameterSerializer<ParameterKey_t>::read_content_from_cdr_message(
        ParameterKey_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_KEY_HASH_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readData(cdr_message, parameter.key.value, PARAMETER_KEY_HASH_LENGTH);
}

template<>
inline uint32_t ParameterSerializer<ParameterString_t>::cdr_serialized_size(
        const ParameterString_t& parameter)
{
    // Size including NUL char at the end
    uint32_t str_siz = static_cast<uint32_t>(parameter.size()) + 1;
    // Align to next 4 byte
    str_siz = (str_siz + 3) & ~3;
    // p_id + p_length + str_length + str_data
    return 2 + 2 + 4 + str_siz;
}

template<>
inline bool ParameterSerializer<ParameterString_t>::add_to_cdr_message(
        const ParameterString_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    if (parameter.size() == 0)
    {
        return false;
    }
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, parameter.Pid);
    //Str size
    uint32_t str_siz = static_cast<uint32_t>(parameter.size() + 1);
    uint16_t len = static_cast<uint16_t>(str_siz + 4 + 3) & ~3;
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, len);
    valid &= fastrtps::rtps::CDRMessage::add_string(cdr_message, parameter.getName());
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterString_t>::read_content_from_cdr_message(
        ParameterString_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length > 256)
    {
        return false;
    }

    parameter.length = parameter_length;
    fastrtps::string_255 aux;
    bool valid = fastrtps::rtps::CDRMessage::readString(cdr_message, &aux);
    parameter.setName(aux.c_str());
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterPort_t>::add_content_to_cdr_message(
        const ParameterPort_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return fastrtps::rtps::CDRMessage::addUInt32(cdr_message, parameter.port);
}

template<>
inline bool ParameterSerializer<ParameterPort_t>::read_content_from_cdr_message(
        ParameterPort_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_PORT_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &parameter.port);
}

template<>
inline bool ParameterSerializer<ParameterGuid_t>::add_content_to_cdr_message(
        const ParameterGuid_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addData(cdr_message, parameter.guid.guidPrefix.value, 12);
    valid &= fastrtps::rtps::CDRMessage::addData(cdr_message, parameter.guid.entityId.value, 4);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterGuid_t>::read_content_from_cdr_message(
        ParameterGuid_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_GUID_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid =  fastrtps::rtps::CDRMessage::readData(cdr_message, parameter.guid.guidPrefix.value, 12);
    valid &= fastrtps::rtps::CDRMessage::readData(cdr_message, parameter.guid.entityId.value, 4);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterProtocolVersion_t>::add_content_to_cdr_message(
        const ParameterProtocolVersion_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, parameter.protocolVersion.m_major);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, parameter.protocolVersion.m_minor);
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, 0);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterProtocolVersion_t>::read_content_from_cdr_message(
        ParameterProtocolVersion_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_PROTOCOL_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message, &parameter.protocolVersion.m_major);
    valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &parameter.protocolVersion.m_minor);
    cdr_message->pos += 2; //padding
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterVendorId_t>::add_content_to_cdr_message(
        const ParameterVendorId_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, parameter.vendorId[0]);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, parameter.vendorId[1]);
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, 0);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterVendorId_t>::read_content_from_cdr_message(
        ParameterVendorId_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_VENDOR_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message, &parameter.vendorId[0]);
    valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &parameter.vendorId[1]);
    cdr_message->pos += 2; //padding
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterIP4Address_t>::add_content_to_cdr_message(
        const ParameterIP4Address_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return fastrtps::rtps::CDRMessage::addData(cdr_message, parameter.address, 4);
}

template<>
inline bool ParameterSerializer<ParameterIP4Address_t>::read_content_from_cdr_message(
        ParameterIP4Address_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_IP4_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readData(cdr_message, parameter.address, 4);
}

template<>
inline bool ParameterSerializer<ParameterBool_t>::add_content_to_cdr_message(
        const ParameterBool_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    fastrtps::rtps::octet val = parameter.value ? 1 : 0;
    bool valid = fastrtps::rtps::CDRMessage::addOctet(cdr_message, val);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, 0);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterBool_t>::read_content_from_cdr_message(
        ParameterBool_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_BOOL_LENGTH)
    {
        return false;
    }

    parameter.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readOctet(cdr_message, (fastrtps::rtps::octet*)&parameter.value);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterStatusInfo_t>::add_content_to_cdr_message(
        const ParameterStatusInfo_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, parameter.status);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterStatusInfo_t>::read_content_from_cdr_message(
        ParameterStatusInfo_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_STATUS_INFO_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    //octet status = msg.buffer[msg.pos + 3];
    bool valid = true;
    fastrtps::rtps::octet tmp;
    //Remove the front three octets, take the fourth
    valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &tmp);
    valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &tmp);
    valid &= fastrtps::rtps::CDRMessage::readOctet(cdr_message, &tmp);
    return fastrtps::rtps::CDRMessage::readOctet(cdr_message, &parameter.status);
}

template<>
inline bool ParameterSerializer<ParameterCount_t>::add_content_to_cdr_message(
        const ParameterCount_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return fastrtps::rtps::CDRMessage::addUInt32(cdr_message, parameter.count);
}

template<>
inline bool ParameterSerializer<ParameterCount_t>::read_content_from_cdr_message(
        ParameterCount_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_COUNT_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &parameter.count);
}

template<>
inline bool ParameterSerializer<ParameterEntityId_t>::add_content_to_cdr_message(
        const ParameterEntityId_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return fastrtps::rtps::CDRMessage::addEntityId(cdr_message, &parameter.entityId);
}

template<>
inline bool ParameterSerializer<ParameterEntityId_t>::read_content_from_cdr_message(
        ParameterEntityId_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_ENTITYID_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readEntityId(cdr_message, &parameter.entityId);
}

template<>
inline bool ParameterSerializer<ParameterTime_t>::add_content_to_cdr_message(
        const ParameterTime_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addInt32(cdr_message, parameter.time.seconds());
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message, parameter.time.fraction());
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterTime_t>::read_content_from_cdr_message(
        ParameterTime_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    int32_t sec(0);
    bool valid = fastrtps::rtps::CDRMessage::readInt32(cdr_message, &sec);
    parameter.time.seconds(sec);
    uint32_t frac(0);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &frac);
    parameter.time.fraction(frac);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterBuiltinEndpointSet_t>::add_content_to_cdr_message(
        const ParameterBuiltinEndpointSet_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    return fastrtps::rtps::CDRMessage::addUInt32(cdr_message, parameter.endpointSet);
}

template<>
inline bool ParameterSerializer<ParameterBuiltinEndpointSet_t>::read_content_from_cdr_message(
        ParameterBuiltinEndpointSet_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_BUILTINENDPOINTSET_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &parameter.endpointSet);
}

template<>
inline uint32_t ParameterSerializer<ParameterPropertyList_t>::cdr_serialized_size(
        const ParameterPropertyList_t& parameter)
{
    // p_id + p_length + n_properties
    uint32_t ret_val = 2 + 2 + 4;
    for (ParameterPropertyList_t::const_iterator it = parameter.begin();
            it != parameter.end(); ++it)
    {
        // str_len + null_char + str_data
        ret_val += 4 + 1 + static_cast<uint32_t>(strlen(it->first().c_str()));
        // align
        ret_val = (ret_val + 3) & ~3;
        // str_len + null_char + str_data
        ret_val += 4 + 1 + static_cast<uint32_t>(strlen(it->second().c_str()));
        // align
        ret_val = (ret_val + 3) & ~3;
    }

    return ret_val;
}

template<>
inline bool ParameterSerializer<ParameterPropertyList_t>::add_to_cdr_message(
        const ParameterPropertyList_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, parameter.Pid);
    uint16_t pos_str = (uint16_t)cdr_message->pos;
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, parameter.length);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message, (uint32_t)parameter.size());
    for (ParameterPropertyList_t::const_iterator it = parameter.begin();
            it != parameter.end(); ++it)
    {
        valid &= fastrtps::rtps::CDRMessage::add_string(cdr_message, it->first());
        valid &= fastrtps::rtps::CDRMessage::add_string(cdr_message, it->second());
    }
    uint16_t pos_param_end = (uint16_t)cdr_message->pos;
    uint16_t len = pos_param_end - pos_str - 2;
    cdr_message->pos = pos_str;
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, len);
    cdr_message->pos = pos_param_end;
    cdr_message->length -= 2;
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterPropertyList_t>::read_content_from_cdr_message(
        ParameterPropertyList_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter.max_size() != 0 && parameter_length > parameter.max_size() + 4)
    {
        return false;
    }
    parameter.length = parameter_length;

    uint32_t pos_ref = cdr_message->pos;
    uint32_t num_properties;
    bool valid = fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &num_properties);

    //properties_.reserve(parameter_length - 4);

    for (size_t i = 0; i < num_properties; ++i)
    {
        uint32_t property1_size = 0, alignment1 = 0, property2_size = 0, alignment2 = 0, str1_pos = 0;

        valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &property1_size);
        if (!valid)
        {
            return false;
        }
        str1_pos = cdr_message->pos;
        alignment1 = ((property1_size + 3) & ~3) - property1_size;
        cdr_message->pos += (property1_size + alignment1);
        valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &property2_size);
        if (!valid)
        {
            return false;
        }
        parameter.push_back(
            &cdr_message->buffer[str1_pos], property1_size,
            &cdr_message->buffer[cdr_message->pos], property2_size);

        alignment2 = ((property2_size + 3) & ~3) - property2_size;
        cdr_message->pos += (property2_size + alignment2);
    }
    //Nproperties_ = num_properties;

    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterSampleIdentity_t>::add_content_to_cdr_message(
        const ParameterSampleIdentity_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addData(cdr_message,
                    parameter.sample_id.writer_guid().guidPrefix.value, fastrtps::rtps::GuidPrefix_t::size);
    valid &= fastrtps::rtps::CDRMessage::addData(cdr_message,
                    parameter.sample_id.writer_guid().entityId.value, fastrtps::rtps::EntityId_t::size);
    valid &= fastrtps::rtps::CDRMessage::addInt32(cdr_message, parameter.sample_id.sequence_number().high);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message, parameter.sample_id.sequence_number().low);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterSampleIdentity_t>::read_content_from_cdr_message(
        ParameterSampleIdentity_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_SAMPLEIDENTITY_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readData(cdr_message,
                    parameter.sample_id.writer_guid().guidPrefix.value, fastrtps::rtps::GuidPrefix_t::size);
    valid &= fastrtps::rtps::CDRMessage::readData(cdr_message,
                    parameter.sample_id.writer_guid().entityId.value, fastrtps::rtps::EntityId_t::size);
    valid &= fastrtps::rtps::CDRMessage::readInt32(cdr_message, &parameter.sample_id.sequence_number().high);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &parameter.sample_id.sequence_number().low);
    return valid;
}

#if HAVE_SECURITY

template<>
inline uint32_t ParameterSerializer<ParameterToken_t>::cdr_serialized_size(
        const ParameterToken_t& parameter)
{
    return ParameterSerializer<Parameter_t>::cdr_serialized_size(parameter.token);
}

template<>
inline bool ParameterSerializer<ParameterToken_t>::add_to_cdr_message(
        const ParameterToken_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt16(cdr_message, parameter.Pid);
    uint16_t pos_str = (uint16_t)cdr_message->pos;
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, parameter.length);
    valid &= fastrtps::rtps::CDRMessage::addDataHolder(cdr_message, parameter.token);
    uint32_t align = (4 - cdr_message->pos % 4) & 3; //align
    for (uint32_t count = 0; count < align; ++count)
    {
        valid &= fastrtps::rtps::CDRMessage::addOctet(cdr_message, 0);
    }
    uint16_t pos_param_end = (uint16_t)cdr_message->pos;
    uint16_t len = pos_param_end - pos_str - 2;
    cdr_message->pos = pos_str;
    valid &= fastrtps::rtps::CDRMessage::addUInt16(cdr_message, len);
    cdr_message->pos = pos_param_end;
    cdr_message->length -= 2;
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterToken_t>::read_content_from_cdr_message(
        ParameterToken_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{

    parameter.length = parameter_length;
    uint32_t pos_ref = cdr_message->pos;
    bool valid =  fastrtps::rtps::CDRMessage::readDataHolder(cdr_message, parameter.token);
    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterParticipantSecurityInfo_t>::add_content_to_cdr_message(
        const ParameterParticipantSecurityInfo_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt32(cdr_message, parameter.security_attributes);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message, parameter.plugin_security_attributes);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterParticipantSecurityInfo_t>::read_content_from_cdr_message(
        ParameterParticipantSecurityInfo_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &parameter.security_attributes);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &parameter.plugin_security_attributes);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterEndpointSecurityInfo_t>::add_content_to_cdr_message(
        const ParameterEndpointSecurityInfo_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message)
{
    bool valid = fastrtps::rtps::CDRMessage::addUInt32(cdr_message, parameter.security_attributes);
    valid &= fastrtps::rtps::CDRMessage::addUInt32(cdr_message, parameter.plugin_security_attributes);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterEndpointSecurityInfo_t>::read_content_from_cdr_message(
        ParameterEndpointSecurityInfo_t& parameter,
        fastrtps::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length != PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &parameter.security_attributes);
    valid &= fastrtps::rtps::CDRMessage::readUInt32(cdr_message, &parameter.plugin_security_attributes);
    return valid;
}

#endif

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_CORE_POLICY__PARAMETERSERIALIZER_HPP_
