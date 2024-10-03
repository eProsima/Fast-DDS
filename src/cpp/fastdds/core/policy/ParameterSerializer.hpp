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

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
namespace eprosima {
namespace fastdds {
namespace dds {

template <typename Parameter>
class ParameterSerializer
{
public:

    static inline bool add_common_to_cdr_message(
            const Parameter& parameter,
            rtps::CDRMessage_t* cdr_message)
    {
        bool valid = rtps::CDRMessage::addUInt16(cdr_message, parameter.Pid);
        valid &= rtps::CDRMessage::addUInt16(cdr_message, parameter.length);
        return valid;
    }

    static bool add_to_cdr_message(
            const Parameter& parameter,
            rtps::CDRMessage_t* cdr_message)
    {
        bool valid = add_common_to_cdr_message(parameter, cdr_message);
        valid &= add_content_to_cdr_message(parameter, cdr_message);
        return valid;
    }

    static bool read_from_cdr_message(
            Parameter& parameter,
            rtps::CDRMessage_t* cdr_message,
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
            rtps::CDRMessage_t*)
    {
        static_assert(sizeof(Parameter) == 0, "Not implemented");
        return false;
    }

    static bool read_content_from_cdr_message(
            Parameter&,
            rtps::CDRMessage_t*,
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

    static constexpr uint32_t PARAMETER_STATUS_SIZE = 8u;
    static constexpr uint32_t PARAMETER_KEY_SIZE = 20u;
    static constexpr uint32_t PARAMETER_SENTINEL_SIZE = 4u;
    static constexpr uint32_t PARAMETER_SAMPLE_IDENTITY_SIZE = 28u;

    static bool add_parameter_status(
            rtps::CDRMessage_t* cdr_message,
            rtps::octet status)
    {
        if (cdr_message->pos + 8 >= cdr_message->max_size)
        {
            return false;
        }
        rtps::CDRMessage::addUInt16(cdr_message, dds::PID_STATUS_INFO);
        rtps::CDRMessage::addUInt16(cdr_message, 4);
        rtps::CDRMessage::addOctet(cdr_message, 0);
        rtps::CDRMessage::addOctet(cdr_message, 0);
        rtps::CDRMessage::addOctet(cdr_message, 0);
        rtps::CDRMessage::addOctet(cdr_message, status);
        return true;
    }

    static bool add_parameter_key(
            rtps::CDRMessage_t* cdr_message,
            const rtps::InstanceHandle_t& iHandle)
    {
        if (cdr_message->pos + 20 >= cdr_message->max_size)
        {
            return false;
        }
        rtps::CDRMessage::addUInt16(cdr_message, dds::PID_KEY_HASH);
        rtps::CDRMessage::addUInt16(cdr_message, 16);
        rtps::CDRMessage::addData(cdr_message, iHandle.value, 16);
        return true;
    }

    static bool add_parameter_sentinel(
            rtps::CDRMessage_t* cdr_message)
    {
        if (cdr_message->pos + 4 > cdr_message->max_size)
        {
            return false;
        }
        rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(dds::PID_SENTINEL));
        rtps::CDRMessage::addUInt16(cdr_message, 0);

        return true;
    }

    /**
     * This method fills the sample identity parameter to the cdr_message.
     * The PID used is the standard PID_RELATED_SAMPLE_IDENTITY.
     * @param cdr_message Message to be filled up.
     * @param sample_id Sample id.
     * @return true if operation is successful, false if the operation would overflow the maximum size of the message.
     */
    static bool add_parameter_sample_identity(
            rtps::CDRMessage_t* cdr_message,
            const rtps::SampleIdentity& sample_id)
    {
        if (cdr_message->pos + 28 > cdr_message->max_size)
        {
            return false;
        }

        rtps::CDRMessage::addUInt16(cdr_message, dds::PID_RELATED_SAMPLE_IDENTITY);
        rtps::CDRMessage::addUInt16(cdr_message, 24);
        rtps::CDRMessage::addData(cdr_message,
                sample_id.writer_guid().guidPrefix.value, rtps::GuidPrefix_t::size);
        rtps::CDRMessage::addData(cdr_message,
                sample_id.writer_guid().entityId.value, rtps::EntityId_t::size);
        rtps::CDRMessage::addInt32(cdr_message, sample_id.sequence_number().high);
        rtps::CDRMessage::addUInt32(cdr_message, sample_id.sequence_number().low);
        return true;
    }

    /**
     * This method fills the sample identity parameter to the cdr_message.
     * The PID used is the legacy PID_RELATED_SAMPLE_IDENTITY: PID_CUSTOM_RELATED_SAMPLE_IDENTITY, due to backwards compatibility compliance.
     * @param cdr_message Message to be filled up.
     * @param sample_id Sample id.
     * @return true if operation is successful, false if the operation would overflow the maximum size of the message.
     */
    static bool add_parameter_custom_related_sample_identity(
            rtps::CDRMessage_t* cdr_message,
            const rtps::SampleIdentity& sample_id)
    {
        if (cdr_message->pos + 28 > cdr_message->max_size)
        {
            return false;
        }

        rtps::CDRMessage::addUInt16(cdr_message, dds::PID_CUSTOM_RELATED_SAMPLE_IDENTITY);
        rtps::CDRMessage::addUInt16(cdr_message, 24);
        rtps::CDRMessage::addData(cdr_message,
                sample_id.writer_guid().guidPrefix.value, rtps::GuidPrefix_t::size);
        rtps::CDRMessage::addData(cdr_message,
                sample_id.writer_guid().entityId.value, rtps::EntityId_t::size);
        rtps::CDRMessage::addInt32(cdr_message, sample_id.sequence_number().high);
        rtps::CDRMessage::addUInt32(cdr_message, sample_id.sequence_number().low);
        return true;
    }

    static inline uint32_t cdr_serialized_size(
            const fastcdr::string_255& str)
    {
        // Size including NUL char at the end
        uint32_t str_siz = static_cast<uint32_t>(str.size()) + 1;
        // Align to next 4 byte
        str_siz = (str_siz + 3u) & ~3u;
        // p_id + p_length + str_length + str_data
        return 2u + 2u + 4u + str_siz;
    }

    static inline uint32_t cdr_serialized_size(
            const rtps::Token& token)
    {
        // p_id + p_length
        uint32_t ret_val = 2 + 2;

        // str_len + null_char + str_data
        ret_val += 4 + 1 + static_cast<uint32_t>(strlen(token.class_id().c_str()));
        // align
        ret_val = (ret_val + 3) & ~3;

        // properties
        ret_val += static_cast<uint32_t>(rtps::PropertyHelper::serialized_size(token.properties()));
        // align
        ret_val = (ret_val + 3) & ~3;

        // binary_properties
        ret_val +=
                static_cast<uint32_t>(rtps::BinaryPropertyHelper::serialized_size(
                    token.binary_properties()));
        // align
        ret_val = (ret_val + 3) & ~3;

        return ret_val;
    }

};

template<>
inline bool ParameterSerializer<ParameterLocator_t>::add_content_to_cdr_message(
        const ParameterLocator_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    return rtps::CDRMessage::addLocator(cdr_message, parameter.locator);
}

template<>
inline bool ParameterSerializer<ParameterLocator_t>::read_content_from_cdr_message(
        ParameterLocator_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_LOCATOR_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return rtps::CDRMessage::readLocator(cdr_message, &parameter.locator);
}

template<>
inline bool ParameterSerializer<ParameterKey_t>::add_to_cdr_message(
        const ParameterKey_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    return ParameterSerializer<Parameter_t>::add_parameter_key(cdr_message, parameter.key);
}

template<>
inline bool ParameterSerializer<ParameterKey_t>::read_content_from_cdr_message(
        ParameterKey_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_KEY_HASH_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return rtps::CDRMessage::readData(cdr_message, parameter.key.value, PARAMETER_KEY_HASH_LENGTH);
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
        rtps::CDRMessage_t* cdr_message)
{
    if (parameter.size() == 0)
    {
        return false;
    }
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, parameter.Pid);
    //Str size
    uint32_t str_siz = static_cast<uint32_t>(parameter.size() + 1);
    uint16_t len = static_cast<uint16_t>(str_siz + 4 + 3) & ~3;
    valid &= rtps::CDRMessage::addUInt16(cdr_message, len);
    valid &= rtps::CDRMessage::add_string(cdr_message, parameter.getName());
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterString_t>::read_content_from_cdr_message(
        ParameterString_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length > 256)
    {
        return false;
    }

    parameter.length = parameter_length;
    fastcdr::string_255 aux;
    bool valid = rtps::CDRMessage::readString(cdr_message, &aux);
    parameter.setName(aux.c_str());
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterPort_t>::add_content_to_cdr_message(
        const ParameterPort_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    return rtps::CDRMessage::addUInt32(cdr_message, parameter.port);
}

template<>
inline bool ParameterSerializer<ParameterPort_t>::read_content_from_cdr_message(
        ParameterPort_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_PORT_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return rtps::CDRMessage::readUInt32(cdr_message, &parameter.port);
}

template<>
inline bool ParameterSerializer<ParameterGuid_t>::add_content_to_cdr_message(
        const ParameterGuid_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addData(cdr_message, parameter.guid.guidPrefix.value, 12);
    valid &= rtps::CDRMessage::addData(cdr_message, parameter.guid.entityId.value, 4);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterGuid_t>::read_content_from_cdr_message(
        ParameterGuid_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_GUID_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid =  rtps::CDRMessage::readData(cdr_message, parameter.guid.guidPrefix.value, 12);
    valid &= rtps::CDRMessage::readData(cdr_message, parameter.guid.entityId.value, 4);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterProtocolVersion_t>::add_content_to_cdr_message(
        const ParameterProtocolVersion_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, parameter.protocolVersion.m_major);
    valid &= rtps::CDRMessage::addOctet(cdr_message, parameter.protocolVersion.m_minor);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 0);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterProtocolVersion_t>::read_content_from_cdr_message(
        ParameterProtocolVersion_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_PROTOCOL_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message, &parameter.protocolVersion.m_major);
    valid &= rtps::CDRMessage::readOctet(cdr_message, &parameter.protocolVersion.m_minor);
    cdr_message->pos += 2; //padding
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterVendorId_t>::add_content_to_cdr_message(
        const ParameterVendorId_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, parameter.vendorId[0]);
    valid &= rtps::CDRMessage::addOctet(cdr_message, parameter.vendorId[1]);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 0);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterVendorId_t>::read_content_from_cdr_message(
        ParameterVendorId_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_VENDOR_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message, &parameter.vendorId[0]);
    valid &= rtps::CDRMessage::readOctet(cdr_message, &parameter.vendorId[1]);
    cdr_message->pos += 2; //padding
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterProductVersion_t>::add_content_to_cdr_message(
        const ParameterProductVersion_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addOctet(cdr_message, parameter.version.major);
    valid &= rtps::CDRMessage::addOctet(cdr_message, parameter.version.minor);
    valid &= rtps::CDRMessage::addOctet(cdr_message, parameter.version.patch);
    valid &= rtps::CDRMessage::addOctet(cdr_message, parameter.version.tweak);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterProductVersion_t>::read_content_from_cdr_message(
        ParameterProductVersion_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_PRODUCT_VERSION_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message, &parameter.version.major);
    valid &= rtps::CDRMessage::readOctet(cdr_message, &parameter.version.minor);
    valid &= rtps::CDRMessage::readOctet(cdr_message, &parameter.version.patch);
    valid &= rtps::CDRMessage::readOctet(cdr_message, &parameter.version.tweak);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterDomainId_t>::add_content_to_cdr_message(
        const ParameterDomainId_t& parameter,
        fastdds::rtps::CDRMessage_t* cdr_message)
{
    return fastdds::rtps::CDRMessage::addUInt32(cdr_message, parameter.domain_id);
}

template<>
inline bool ParameterSerializer<ParameterDomainId_t>::read_content_from_cdr_message(
        ParameterDomainId_t& parameter,
        fastdds::rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_DOMAINID_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return fastdds::rtps::CDRMessage::readUInt32(cdr_message, &parameter.domain_id);
}

template<>
inline bool ParameterSerializer<ParameterIP4Address_t>::add_content_to_cdr_message(
        const ParameterIP4Address_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    return rtps::CDRMessage::addData(cdr_message, parameter.address, 4);
}

template<>
inline bool ParameterSerializer<ParameterIP4Address_t>::read_content_from_cdr_message(
        ParameterIP4Address_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_IP4_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return rtps::CDRMessage::readData(cdr_message, parameter.address, 4);
}

template<>
inline bool ParameterSerializer<ParameterBool_t>::add_content_to_cdr_message(
        const ParameterBool_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    rtps::octet val = parameter.value ? 1 : 0;
    bool valid = rtps::CDRMessage::addOctet(cdr_message, val);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addUInt16(cdr_message, 0);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterBool_t>::read_content_from_cdr_message(
        ParameterBool_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_BOOL_LENGTH)
    {
        return false;
    }

    parameter.length = parameter_length;
    bool valid = rtps::CDRMessage::readOctet(cdr_message, (rtps::octet*)&parameter.value);
    cdr_message->pos += 3; //padding
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterStatusInfo_t>::add_content_to_cdr_message(
        const ParameterStatusInfo_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    valid &= rtps::CDRMessage::addOctet(cdr_message, parameter.status);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterStatusInfo_t>::read_content_from_cdr_message(
        ParameterStatusInfo_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_STATUS_INFO_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    //octet status = msg.buffer[msg.pos + 3];
    rtps::octet tmp;
    //Remove the front three octets, take the fourth
    bool valid = rtps::CDRMessage::readOctet(cdr_message, &tmp);
    valid &= rtps::CDRMessage::readOctet(cdr_message, &tmp);
    valid &= rtps::CDRMessage::readOctet(cdr_message, &tmp);
    valid &= rtps::CDRMessage::readOctet(cdr_message, &parameter.status);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterCount_t>::add_content_to_cdr_message(
        const ParameterCount_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    return rtps::CDRMessage::addUInt32(cdr_message, parameter.count);
}

template<>
inline bool ParameterSerializer<ParameterCount_t>::read_content_from_cdr_message(
        ParameterCount_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_COUNT_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return rtps::CDRMessage::readUInt32(cdr_message, &parameter.count);
}

template<>
inline bool ParameterSerializer<ParameterEntityId_t>::add_content_to_cdr_message(
        const ParameterEntityId_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    return rtps::CDRMessage::addEntityId(cdr_message, &parameter.entityId);
}

template<>
inline bool ParameterSerializer<ParameterEntityId_t>::read_content_from_cdr_message(
        ParameterEntityId_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_ENTITYID_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return rtps::CDRMessage::readEntityId(cdr_message, &parameter.entityId);
}

template<>
inline bool ParameterSerializer<ParameterTime_t>::add_content_to_cdr_message(
        const ParameterTime_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addInt32(cdr_message, parameter.time.seconds());
    valid &= rtps::CDRMessage::addInt32(cdr_message, parameter.time.fraction());
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterTime_t>::read_content_from_cdr_message(
        ParameterTime_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    int32_t sec(0);
    bool valid = rtps::CDRMessage::readInt32(cdr_message, &sec);
    parameter.time.seconds(sec);
    uint32_t frac(0);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &frac);
    parameter.time.fraction(frac);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterBuiltinEndpointSet_t>::add_content_to_cdr_message(
        const ParameterBuiltinEndpointSet_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    return rtps::CDRMessage::addUInt32(cdr_message, parameter.endpointSet);
}

template<>
inline bool ParameterSerializer<ParameterBuiltinEndpointSet_t>::read_content_from_cdr_message(
        ParameterBuiltinEndpointSet_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_BUILTINENDPOINTSET_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return rtps::CDRMessage::readUInt32(cdr_message, &parameter.endpointSet);
}

template<>
inline bool ParameterSerializer<ParameterNetworkConfigSet_t>::add_content_to_cdr_message(
        const ParameterNetworkConfigSet_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    return rtps::CDRMessage::addUInt32(cdr_message, parameter.netconfigSet);
}

template<>
inline bool ParameterSerializer<ParameterNetworkConfigSet_t>::read_content_from_cdr_message(
        ParameterNetworkConfigSet_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_NETWORKCONFIGSET_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    return rtps::CDRMessage::readUInt32(cdr_message, &parameter.netconfigSet);
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
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, parameter.Pid);
    uint16_t pos_str = (uint16_t)cdr_message->pos;
    valid &= rtps::CDRMessage::addUInt16(cdr_message, parameter.length);
    valid &= rtps::CDRMessage::addUInt32(cdr_message, (uint32_t)parameter.size());
    for (ParameterPropertyList_t::const_iterator it = parameter.begin();
            it != parameter.end(); ++it)
    {
        valid &= rtps::CDRMessage::add_string(cdr_message, it->first());
        valid &= rtps::CDRMessage::add_string(cdr_message, it->second());
    }
    uint16_t pos_param_end = (uint16_t)cdr_message->pos;
    uint16_t len = pos_param_end - pos_str - 2;
    cdr_message->pos = pos_str;
    valid &= rtps::CDRMessage::addUInt16(cdr_message, len);
    cdr_message->pos = pos_param_end;
    cdr_message->length -= 2;
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterPropertyList_t>::read_content_from_cdr_message(
        ParameterPropertyList_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter.max_size() != 0 && parameter_length > parameter.max_size() + 4)
    {
        return false;
    }
    parameter.length = parameter_length;

    uint32_t pos_ref = cdr_message->pos;
    uint32_t max_pos = pos_ref + parameter_length;
    uint32_t remain = parameter_length;
    if ((max_pos > cdr_message->length) || (remain < sizeof(uint32_t)))
    {
        return false;
    }

    uint32_t num_properties = 0;
    bool valid = rtps::CDRMessage::readUInt32(cdr_message, &num_properties);
    remain -= sizeof(uint32_t);
    if (!valid)
    {
        return false;
    }

    for (uint32_t i = 0; i < num_properties; ++i)
    {
        uint32_t property1_size = 0, alignment1 = 0, property2_size = 0, alignment2 = 0, str1_pos = 0, str2_pos = 0;

        // Read and validate size of property name
        remain = max_pos - cdr_message->pos;
        valid &= (remain >= sizeof(uint32_t)) && rtps::CDRMessage::readUInt32(cdr_message, &property1_size);
        remain -= sizeof(uint32_t);
        valid = valid && (remain >= property1_size);
        if (!valid)
        {
            return false;
        }

        str1_pos = cdr_message->pos;
        cdr_message->pos += property1_size;
        remain -= property1_size;
        alignment1 = ((property1_size + 3u) & ~3u) - property1_size;
        if (remain < alignment1)
        {
            return false;
        }
        cdr_message->pos += alignment1;
        remain -= alignment1;

        // Read and validate size of property value
        valid &= (remain >= sizeof(uint32_t)) && rtps::CDRMessage::readUInt32(cdr_message, &property2_size);
        remain -= sizeof(uint32_t);
        valid = valid && (remain >= property2_size);
        if (!valid)
        {
            return false;
        }

        str2_pos = cdr_message->pos;
        cdr_message->pos += property2_size;
        remain -= property2_size;
        alignment2 = ((property2_size + 3u) & ~3u) - property2_size;
        if (remain < alignment2)
        {
            return false;
        }
        cdr_message->pos += alignment2;
        remain -= alignment2;

        parameter.push_back(
            &cdr_message->buffer[str1_pos], property1_size,
            &cdr_message->buffer[str2_pos], property2_size);
    }

    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length >= length_diff);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterSampleIdentity_t>::add_content_to_cdr_message(
        const ParameterSampleIdentity_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addData(cdr_message,
                    parameter.sample_id.writer_guid().guidPrefix.value, rtps::GuidPrefix_t::size);
    valid &= rtps::CDRMessage::addData(cdr_message,
                    parameter.sample_id.writer_guid().entityId.value, rtps::EntityId_t::size);
    valid &= rtps::CDRMessage::addInt32(cdr_message, parameter.sample_id.sequence_number().high);
    valid &= rtps::CDRMessage::addUInt32(cdr_message, parameter.sample_id.sequence_number().low);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterSampleIdentity_t>::read_content_from_cdr_message(
        ParameterSampleIdentity_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_SAMPLEIDENTITY_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = rtps::CDRMessage::readData(cdr_message,
                    parameter.sample_id.writer_guid().guidPrefix.value, rtps::GuidPrefix_t::size);
    valid &= rtps::CDRMessage::readData(cdr_message,
                    parameter.sample_id.writer_guid().entityId.value, rtps::EntityId_t::size);
    valid &= rtps::CDRMessage::readInt32(cdr_message, &parameter.sample_id.sequence_number().high);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &parameter.sample_id.sequence_number().low);
    return valid;
}

template<>
class ParameterSerializer<rtps::ContentFilterProperty>
{
public:

    static uint32_t cdr_serialized_size(
            const rtps::ContentFilterProperty& parameter)
    {
        uint32_t ret_val = 0;

        if (0 < parameter.filter_class_name.size() &&
                0 < parameter.content_filtered_topic_name.size() &&
                0 < parameter.related_topic_name.size() &&
                0 < parameter.filter_expression.size())
        {
            // p_id + p_length
            ret_val = 2 + 2;
            // content_filtered_topic_name
            ret_val += cdr_serialized_size(parameter.content_filtered_topic_name);
            // related_topic_name
            ret_val += cdr_serialized_size(parameter.related_topic_name);
            // filter_class_name
            ret_val += cdr_serialized_size(parameter.filter_class_name);

            // filter_expression
            // str_len + null_char + str_data
            ret_val += 4 + 1 + static_cast<uint32_t>(parameter.filter_expression.size());
            // align
            ret_val = (ret_val + 3) & ~3;

            // expression_parameters
            // sequence length
            ret_val += 4;
            // Add all parameters
            for (const fastcdr::string_255& param : parameter.expression_parameters)
            {
                ret_val += cdr_serialized_size(param);
            }
        }

        return ret_val;
    }

    static bool add_to_cdr_message(
            const rtps::ContentFilterProperty& parameter,
            rtps::CDRMessage_t* cdr_message)
    {
        bool valid = false;

        if (0 < parameter.filter_class_name.size() &&
                0 < parameter.content_filtered_topic_name.size() &&
                0 < parameter.related_topic_name.size() &&
                0 < parameter.filter_expression.size())
        {
            // p_id + p_length
            uint32_t len = cdr_serialized_size(parameter);
            assert(4 < len && 0xFFFF >= (len - 4));
            valid = rtps::CDRMessage::addUInt16(cdr_message, PID_CONTENT_FILTER_PROPERTY);
            valid &= rtps::CDRMessage::addUInt16(cdr_message, static_cast<uint16_t>(len - 4));
            // content_filtered_topic_name
            valid &= rtps::CDRMessage::add_string(cdr_message, parameter.content_filtered_topic_name);
            // related_topic_name
            valid &= rtps::CDRMessage::add_string(cdr_message, parameter.related_topic_name);
            // filter_class_name
            valid &= rtps::CDRMessage::add_string(cdr_message, parameter.filter_class_name);
            // filter_expression
            valid &= rtps::CDRMessage::add_string(cdr_message, parameter.filter_expression);

            // expression_parameters
            // sequence length
            uint32_t num_params = static_cast<uint32_t>(parameter.expression_parameters.size());
            valid &= rtps::CDRMessage::addUInt32(cdr_message, num_params);
            // Add all parameters
            for (const fastcdr::string_255& param : parameter.expression_parameters)
            {
                valid &= rtps::CDRMessage::add_string(cdr_message, param);
            }
        }

        return valid;
    }

    static bool read_from_cdr_message(
            rtps::ContentFilterProperty& parameter,
            rtps::CDRMessage_t* cdr_message,
            const uint16_t parameter_length)
    {
        // Ensure incorrect length will result in parameter being cleared
        clear(parameter);

        // Validate minimum plength: 4 non-empty strings + number of expression parameters
        constexpr uint16_t min_plength = (4 * 8) + 4;
        if (parameter_length >= min_plength && (cdr_message->length - cdr_message->pos) > parameter_length)
        {
            bool valid = true;
            // Limit message length to parameter length, keeping old length to restore it later
            uint32_t old_msg_len = cdr_message->length;
            cdr_message->length = cdr_message->pos + parameter_length;

            // Read four strings
            valid = read_string(cdr_message, parameter.content_filtered_topic_name) &&
                    (0 < parameter.content_filtered_topic_name.size());
            if (valid)
            {
                valid = read_string(cdr_message, parameter.related_topic_name) &&
                        (0 < parameter.related_topic_name.size());
            }
            if (valid)
            {
                valid = read_string(cdr_message, parameter.filter_class_name) &&
                        (0 < parameter.filter_class_name.size());
            }
            if (valid)
            {
                valid = rtps::CDRMessage::readString(cdr_message, &parameter.filter_expression) &&
                        (0 < parameter.filter_expression.size());
            }

            // Read parameter sequence
            if (valid)
            {
                uint32_t num_parameters = 0;
                valid = rtps::CDRMessage::readUInt32(cdr_message, &num_parameters);
                if (valid)
                {
                    valid = (num_parameters <= 100) && (num_parameters <= parameter.expression_parameters.max_size());
                }
                if (valid)
                {
                    for (uint32_t i = 0; valid && i < num_parameters; ++i)
                    {
                        fastcdr::string_255* p = parameter.expression_parameters.push_back({});
                        assert(nullptr != p);
                        valid = read_string(cdr_message, *p);
                    }
                }
            }

            cdr_message->length = old_msg_len;
            if (!valid)
            {
                clear(parameter);
            }

            return true;
        }

        return false;
    }

private:

    static inline uint32_t cdr_serialized_size(
            const fastcdr::string_255& str)
    {
        // Size including NUL char at the end
        uint32_t str_siz = static_cast<uint32_t>(str.size()) + 1;
        // Align to next 4 byte
        str_siz = (str_siz + 3u) & ~3u;
        // str_length + str_data
        return 4u + str_siz;
    }

    static inline void clear(
            rtps::ContentFilterProperty& parameter)
    {
        parameter.filter_class_name = "";
        parameter.content_filtered_topic_name = "";
        parameter.related_topic_name = "";
        parameter.filter_expression = "";
        parameter.expression_parameters.clear();
    }

    static inline bool read_string(
            rtps::CDRMessage_t* cdr_message,
            fastcdr::string_255& str)
    {
        uint32_t str_size = 0;
        bool valid;
        valid = rtps::CDRMessage::readUInt32(cdr_message, &str_size);
        if (!valid ||
                cdr_message->pos + str_size > cdr_message->length ||
                str_size > str.max_size + 1)
        {
            return false;
        }

        str = "";
        // str_size == 1 would be for an empty string, as the NUL char is always serialized
        if (str_size > 1)
        {
            str = reinterpret_cast<const char*>(&(cdr_message->buffer[cdr_message->pos]));
        }
        cdr_message->pos += str_size;
        cdr_message->pos = (cdr_message->pos + 3u) & ~3u;

        return true;
    }

};

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
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt16(cdr_message, parameter.Pid);
    uint16_t pos_str = (uint16_t)cdr_message->pos;
    valid &= rtps::CDRMessage::addUInt16(cdr_message, parameter.length);
    valid &= rtps::CDRMessage::addDataHolder(cdr_message, parameter.token);
    uint32_t align = (4 - cdr_message->pos % 4) & 3; //align
    for (uint32_t count = 0; count < align; ++count)
    {
        valid &= rtps::CDRMessage::addOctet(cdr_message, 0);
    }
    uint16_t pos_param_end = (uint16_t)cdr_message->pos;
    uint16_t len = pos_param_end - pos_str - 2;
    cdr_message->pos = pos_str;
    valid &= rtps::CDRMessage::addUInt16(cdr_message, len);
    cdr_message->pos = pos_param_end;
    cdr_message->length -= 2;
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterToken_t>::read_content_from_cdr_message(
        ParameterToken_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{

    parameter.length = parameter_length;
    uint32_t pos_ref = cdr_message->pos;
    bool valid =  rtps::CDRMessage::readDataHolder(cdr_message, parameter.token, parameter_length);
    uint32_t length_diff = cdr_message->pos - pos_ref;
    valid &= (parameter_length == length_diff);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterParticipantSecurityInfo_t>::add_content_to_cdr_message(
        const ParameterParticipantSecurityInfo_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt32(cdr_message, parameter.security_attributes);
    valid &= rtps::CDRMessage::addUInt32(cdr_message, parameter.plugin_security_attributes);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterParticipantSecurityInfo_t>::read_content_from_cdr_message(
        ParameterParticipantSecurityInfo_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = rtps::CDRMessage::readUInt32(cdr_message, &parameter.security_attributes);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &parameter.plugin_security_attributes);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterEndpointSecurityInfo_t>::add_content_to_cdr_message(
        const ParameterEndpointSecurityInfo_t& parameter,
        rtps::CDRMessage_t* cdr_message)
{
    bool valid = rtps::CDRMessage::addUInt32(cdr_message, parameter.security_attributes);
    valid &= rtps::CDRMessage::addUInt32(cdr_message, parameter.plugin_security_attributes);
    return valid;
}

template<>
inline bool ParameterSerializer<ParameterEndpointSecurityInfo_t>::read_content_from_cdr_message(
        ParameterEndpointSecurityInfo_t& parameter,
        rtps::CDRMessage_t* cdr_message,
        const uint16_t parameter_length)
{
    if (parameter_length < PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH)
    {
        return false;
    }
    parameter.length = parameter_length;
    bool valid = rtps::CDRMessage::readUInt32(cdr_message, &parameter.security_attributes);
    valid &= rtps::CDRMessage::readUInt32(cdr_message, &parameter.plugin_security_attributes);
    return valid;
}

#endif // if HAVE_SECURITY

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_CORE_POLICY__PARAMETERSERIALIZER_HPP_
