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

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/messages/CDRMessage.h>

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace dds {

// PARAMETER LOCATOR
bool ParameterLocator_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_LOCATOR_LENGTH);
    valid &= CDRMessage::addLocator(msg, locator);
    return valid;
}

bool ParameterLocator_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_LOCATOR_LENGTH)
    {
        return false;
    }
    length = size;
    return CDRMessage::readLocator(msg, &locator);
}

//PARAMTERKEY
bool ParameterKey_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    return CDRMessage::addParameterKey(msg, &this->key);

}

bool ParameterKey_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_KEY_HASH_LENGTH)
    {
        return false;
    }
    length = size;
    return CDRMessage::readData(msg, key.value, PARAMETER_KEY_HASH_LENGTH);
}

// PARAMETER_ STRING
uint32_t ParameterString_t::cdr_serialized_size(
        const fastrtps::string_255& str)
{
    // Size including NUL char at the end
    uint32_t str_siz = static_cast<uint32_t>(str.size()) + 1;
    // Align to next 4 byte
    str_siz = (str_siz + 3) & ~3;
    // p_id + p_length + str_length + str_data
    return 2 + 2 + 4 + str_siz;
}

bool ParameterString_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    if (string_.size() == 0)
    {
        return false;
    }
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    //Str size
    uint32_t str_siz = static_cast<uint32_t>(string_.size() + 1);
    uint16_t len = static_cast<uint16_t>(str_siz + 4 + 3) & ~3;
    valid &= CDRMessage::addUInt16(msg, len);
    valid &= CDRMessage::add_string(msg, string_);
    return valid;
}

bool ParameterString_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size > 256)
    {
        return false;
    }

    length = size;
    fastrtps::string_255 aux;
    bool valid = CDRMessage::readString(msg, &aux);
    setName(aux.c_str());
    return valid;
}

// PARAMETER_ PORT
bool ParameterPort_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PORT_LENGTH);
    valid &= CDRMessage::addUInt32(msg, this->port);
    return valid;
}

bool ParameterPort_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_PORT_LENGTH)
    {
        return false;
    }
    length = size;
    return CDRMessage::readUInt32(msg, &port);
}

//PARAMETER_ GUID
bool ParameterGuid_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_GUID_LENGTH);
    valid &= CDRMessage::addData(msg, this->guid.guidPrefix.value, 12);
    valid &= CDRMessage::addData(msg, this->guid.entityId.value, 4);
    return valid;
}

bool ParameterGuid_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_GUID_LENGTH)
    {
        return false;
    }
    length = size;
    bool valid =  CDRMessage::readData(msg, guid.guidPrefix.value, 12);
    valid &= CDRMessage::readData(msg, guid.entityId.value, 4);
    return valid;
}

//PARAMETER_ PROTOCOL VERSION
bool ParameterProtocolVersion_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PROTOCOL_LENGTH);
    valid &= CDRMessage::addOctet(msg, protocolVersion.m_major);
    valid &= CDRMessage::addOctet(msg, protocolVersion.m_minor);
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterProtocolVersion_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_PROTOCOL_LENGTH)
    {
        return false;
    }
    length = size;
    bool valid = CDRMessage::readOctet(msg, &protocolVersion.m_major);
    valid &= CDRMessage::readOctet(msg, &protocolVersion.m_minor);
    msg->pos += 2; //padding
    return valid;
}

//PARAMETER_ VENDORID
bool ParameterVendorId_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_VENDOR_LENGTH);
    valid &= CDRMessage::addOctet(msg, vendorId[0]);
    valid &= CDRMessage::addOctet(msg, vendorId[1]);
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterVendorId_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_VENDOR_LENGTH)
    {
        return false;
    }
    length = size;
    bool valid = CDRMessage::readOctet(msg, &vendorId[0]);
    valid &= CDRMessage::readOctet(msg, &vendorId[1]);
    msg->pos += 2; //padding
    return valid;
}

//PARAMETER_ IP4ADDRESS
bool ParameterIP4Address_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_IP4_LENGTH);
    valid &= CDRMessage::addData(msg, this->address, 4);
    return valid;
}

bool ParameterIP4Address_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_IP4_LENGTH)
    {
        return false;
    }
    length = size;
    return CDRMessage::readData(msg, address, 4);
}

void ParameterIP4Address_t::setIP4Address(
        octet o1,
        octet o2,
        octet o3,
        octet o4)
{
    address[0] = o1;
    address[1] = o2;
    address[2] = o3;
    address[3] = o4;
}

bool ParameterBool_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_BOOL_LENGTH);
    octet val = value ? 1 : 0;
    valid &= CDRMessage::addOctet(msg, val);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterBool_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_BOOL_LENGTH)
    {
        return false;
    }

    length = size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&value);
    msg->pos += 3; //padding
    return valid;
}

bool ParameterStatusInfo_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_STATUS_INFO_LENGTH);
    valid &= CDRMessage::addUInt16(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, status);
    return valid;
}

bool ParameterStatusInfo_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_STATUS_INFO_LENGTH)
    {
        return false;
    }
    length = size;
    //octet status = msg.buffer[msg.pos + 3];
    bool valid = true;
    octet tmp;
    //Remove the front three octets, take the fourth
    valid &= CDRMessage::readOctet(msg, &tmp);
    valid &= CDRMessage::readOctet(msg, &tmp);
    valid &= CDRMessage::readOctet(msg, &tmp);
    return CDRMessage::readOctet(msg, &status);
}

bool ParameterCount_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_COUNT_LENGTH);
    valid &= CDRMessage::addUInt32(msg, count);
    return valid;
}

bool ParameterCount_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_COUNT_LENGTH)
    {
        return false;
    }
    length = size;
    return CDRMessage::readUInt32(msg, &count);
}

bool ParameterEntityId_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_ENTITYID_LENGTH);
    valid &= CDRMessage::addEntityId(msg, &entityId);
    return valid;
}

bool ParameterEntityId_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_ENTITYID_LENGTH)
    {
        return false;
    }
    length = size;
    return CDRMessage::readEntityId(msg, &entityId);
}

bool ParameterTime_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_TIME_LENGTH);
    valid &= CDRMessage::addInt32(msg, time.seconds());
    valid &= CDRMessage::addInt32(msg, time.fraction());
    return valid;
}

bool ParameterTime_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_TIME_LENGTH)
    {
        return false;
    }
    length = size;
    int32_t sec(0);
    bool valid = CDRMessage::readInt32(msg, &sec);
    time.seconds(sec);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    time.fraction(frac);
    return valid;
}

bool ParameterBuiltinEndpointSet_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_BUILTINENDPOINTSET_LENGTH);
    valid &= CDRMessage::addUInt32(msg, this->endpointSet);
    return valid;
}

bool ParameterBuiltinEndpointSet_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_BUILTINENDPOINTSET_LENGTH)
    {
        return false;
    }
    length = size;
    return CDRMessage::readUInt32(msg, &endpointSet);
}

uint32_t ParameterPropertyList_t::cdr_serialized_size(
        const ParameterPropertyList_t& data)
{
    // p_id + p_length + n_properties
    uint32_t ret_val = 2 + 2 + 4;
    for (ParameterPropertyList_t::const_iterator it = data.begin();
            it != data.end(); ++it)
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

bool ParameterPropertyList_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    uint16_t pos_str = (uint16_t)msg->pos;
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg, (uint32_t)this->size());
    for (ParameterPropertyList_t::const_iterator it = this->begin();
            it != this->end(); ++it)
    {
        valid &= CDRMessage::add_string(msg, it->first());
        valid &= CDRMessage::add_string(msg, it->second());
    }
    uint16_t pos_param_end = (uint16_t)msg->pos;
    uint16_t len = pos_param_end - pos_str - 2;
    msg->pos = pos_str;
    valid &= CDRMessage::addUInt16(msg, len);
    msg->pos = pos_param_end;
    msg->length -= 2;
    return valid;
}

bool ParameterPropertyList_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (limit_size_ && size > properties_.max_size + 4)
    {
        return false;
    }
    length = size;

    uint32_t pos_ref = msg->pos;
    uint32_t num_properties;
    bool valid = CDRMessage::readUInt32(msg, &num_properties);

    properties_.reserve(size - 4);

    for (size_t i = 0; i < num_properties; ++i)
    {
        uint32_t property_size, alignment;

        valid &= CDRMessage::readUInt32(msg, &property_size);
        if (!valid)
        {
            return false;
        }
        alignment = ((property_size + 3) & ~3) - property_size;
        push_back_helper (&msg->buffer[msg->pos], property_size, alignment);
        msg->pos += (property_size + alignment);

        valid &= CDRMessage::readUInt32(msg, &property_size);
        if (!valid)
        {
            return false;
        }
        alignment = ((property_size + 3) & ~3) - property_size;
        push_back_helper (&msg->buffer[msg->pos], property_size, alignment);
        msg->pos += (property_size + alignment);
    }
    Nproperties_ = num_properties;

    uint32_t length_diff = msg->pos - pos_ref;
    valid &= (size == length_diff);
    return valid;
}

bool ParameterSampleIdentity_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addData(msg, sample_id.writer_guid().guidPrefix.value, GuidPrefix_t::size);
    valid &= CDRMessage::addData(msg, sample_id.writer_guid().entityId.value, EntityId_t::size);
    valid &= CDRMessage::addInt32(msg, sample_id.sequence_number().high);
    valid &= CDRMessage::addUInt32(msg, sample_id.sequence_number().low);
    return valid;
}

bool ParameterSampleIdentity_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_SAMPLEIDENTITY_LENGTH)
    {
        return false;
    }
    length = size;
    bool valid = CDRMessage::readData(msg, sample_id.writer_guid().guidPrefix.value, GuidPrefix_t::size);
    valid &= CDRMessage::readData(msg, sample_id.writer_guid().entityId.value, EntityId_t::size);
    valid &= CDRMessage::readInt32(msg, &sample_id.sequence_number().high);
    valid &= CDRMessage::readUInt32(msg, &sample_id.sequence_number().low);
    return valid;
}

#if HAVE_SECURITY

uint32_t ParameterToken_t::cdr_serialized_size(
        const fastrtps::rtps::Token& data)
{
    // p_id + p_length
    uint32_t ret_val = 2 + 2;

    // str_len + null_char + str_data
    ret_val += 4 + 1 + static_cast<uint32_t>(strlen(data.class_id().c_str()));
    // align
    ret_val = (ret_val + 3) & ~3;

    // properties
    ret_val += static_cast<uint32_t>(PropertyHelper::serialized_size(data.properties()));
    // align
    ret_val = (ret_val + 3) & ~3;

    // binary_properties
    ret_val += static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(data.binary_properties()));
    // align
    ret_val = (ret_val + 3) & ~3;

    return ret_val;
}

bool ParameterToken_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    uint16_t pos_str = (uint16_t)msg->pos;
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addDataHolder(msg, this->token);
    uint32_t align = (4 - msg->pos % 4) & 3; //align
    for (uint32_t count = 0; count < align; ++count)
    {
        valid &= CDRMessage::addOctet(msg, 0);
    }
    uint16_t pos_param_end = (uint16_t)msg->pos;
    uint16_t len = pos_param_end - pos_str - 2;
    msg->pos = pos_str;
    valid &= CDRMessage::addUInt16(msg, len);
    msg->pos = pos_param_end;
    msg->length -= 2;
    return valid;
}

bool ParameterToken_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{

    length = size;
    uint32_t pos_ref = msg->pos;
    bool valid =  CDRMessage::readDataHolder(msg, token);
    uint32_t length_diff = msg->pos - pos_ref;
    valid &= (size == length_diff);
    return valid;
}

bool ParameterParticipantSecurityInfo_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH);
    valid &= CDRMessage::addUInt32(msg, this->security_attributes);
    valid &= CDRMessage::addUInt32(msg, this->plugin_security_attributes);
    return valid;
}

bool ParameterParticipantSecurityInfo_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH)
    {
        return false;
    }
    length = size;
    bool valid = CDRMessage::readUInt32(msg, &security_attributes);
    valid &= CDRMessage::readUInt32(msg, &plugin_security_attributes);
    return valid;
}

bool ParameterEndpointSecurityInfo_t::addToCDRMessage(
        CDRMessage_t* msg) const
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH);
    valid &= CDRMessage::addUInt32(msg, this->security_attributes);
    valid &= CDRMessage::addUInt32(msg, this->plugin_security_attributes);
    return valid;
}

bool ParameterEndpointSecurityInfo_t::readFromCDRMessage(
        CDRMessage_t* msg,
        uint16_t size)
{
    if (size != PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH)
    {
        return false;
    }
    length = size;
    bool valid = CDRMessage::readUInt32(msg, &security_attributes);
    valid &= CDRMessage::readUInt32(msg, &plugin_security_attributes);
    return valid;
}

#endif

} //namespace dds
} //namespace fastdds
} //namespace eprosima
