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
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_LOCATOR_LENGTH);//this->length);
    valid &= CDRMessage::addLocator(msg, &this->locator);
    return valid;
}

//PARAMTERKEY
bool ParameterKey_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    return CDRMessage::addParameterKey(msg, &this->key);

}

// PARAMETER_ STRING
bool ParameterString_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    if (this->string_.size() == 0)
    {
        return false;
    }
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    //Str size
    uint32_t str_siz = (uint32_t)this->string_.size();
    int rest = (str_siz + 1) % 4;
    if (rest != 0)
    {
        rest = 4 - rest; //how many you have to add
    }
    this->length = (uint16_t)(str_siz + 1 + 4 + rest);
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg, str_siz + 1);
    valid &= CDRMessage::addData(msg, (unsigned char*) this->string_.c_str(), str_siz + 1);
    if (rest != 0)
    {
        octet oc = '\0';
        for (int i = 0; i < rest; i++)
        {
            valid &= CDRMessage::addOctet(msg, oc);
        }
    }
    return valid;
}

// PARAMETER_ PORT
bool ParameterPort_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PORT_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg, this->port);
    return valid;
}

//PARAMETER_ GUID
bool ParameterGuid_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_GUID_LENGTH);//this->length);
    valid &= CDRMessage::addData(msg, this->guid.guidPrefix.value, 12);
    valid &= CDRMessage::addData(msg, this->guid.entityId.value, 4);
    return valid;
}

//PARAMETER_ PROTOCOL VERSION
bool ParameterProtocolVersion_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PROTOCOL_LENGTH);//this->length);
    valid &= CDRMessage::addOctet(msg, protocolVersion.m_major);
    valid &= CDRMessage::addOctet(msg, protocolVersion.m_minor);
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterVendorId_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_VENDOR_LENGTH);//this->length);
    valid &= CDRMessage::addOctet(msg, vendorId[0]);
    valid &= CDRMessage::addOctet(msg, vendorId[1]);
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

//PARAMETER_ IP4ADDRESS
bool ParameterIP4Address_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_IP4_LENGTH);//this->length);
    valid &= CDRMessage::addData(msg, this->address, 4);
    return valid;
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
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_BOOL_LENGTH);//this->length);
    octet val = value ? 1 : 0;
    valid &= CDRMessage::addOctet(msg, val);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterStatusInfo_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_STATUS_INFO_LENGTH);//this->length);
    valid &= CDRMessage::addUInt16(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, status);
    return valid;
}

bool ParameterCount_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_COUNT_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg, count);
    return valid;
}

bool ParameterEntityId_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_ENTITYID_LENGTH);//this->length);
    valid &= CDRMessage::addEntityId(msg, &entityId);
    return valid;
}

bool ParameterTime_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_TIME_LENGTH);//this->length);
    valid &= CDRMessage::addInt32(msg, time.seconds());
    valid &= CDRMessage::addInt32(msg, time.fraction());
    return valid;
}

bool ParameterBuiltinEndpointSet_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_BUILTINENDPOINTSET_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg, this->endpointSet);
    return valid;
}

bool ParameterPropertyList_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    uint16_t pos_str = (uint16_t)msg->pos;
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addUInt32(msg, (uint32_t)this->properties.size());
    for (std::vector<std::pair<std::string, std::string> >::iterator it = this->properties.begin();
            it != this->properties.end(); ++it)
    {
        valid &= CDRMessage::addString(msg, it->first);
        valid &= CDRMessage::addString(msg, it->second);
    }
    uint32_t align = (4 - msg->pos % 4) & 3; //align
    for (uint32_t count = 0; count < align; ++count)
    {
        valid &= CDRMessage::addOctet(msg, 0);
    }
    uint16_t pos_param_end = (uint16_t)msg->pos;
    this->length = pos_param_end - pos_str - 2;
    msg->pos = pos_str;
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    msg->pos = pos_param_end;
    msg->length -= 2;
    return valid;
}

bool ParameterSampleIdentity_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addData(msg, sample_id.writer_guid().guidPrefix.value, GuidPrefix_t::size);
    valid &= CDRMessage::addData(msg, sample_id.writer_guid().entityId.value, EntityId_t::size);
    valid &= CDRMessage::addInt32(msg, sample_id.sequence_number().high);
    valid &= CDRMessage::addUInt32(msg, sample_id.sequence_number().low);
    return valid;
}

#if HAVE_SECURITY

bool ParameterToken_t::addToCDRMessage(
        CDRMessage_t* msg)
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
    this->length = pos_param_end - pos_str - 2;
    msg->pos = pos_str;
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    msg->pos = pos_param_end;
    msg->length -= 2;
    return valid;
}

bool ParameterParticipantSecurityInfo_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg, this->security_attributes);
    valid &= CDRMessage::addUInt32(msg, this->plugin_security_attributes);
    return valid;
}

bool ParameterEndpointSecurityInfo_t::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg, this->security_attributes);
    valid &= CDRMessage::addUInt32(msg, this->plugin_security_attributes);
    return valid;
}

#endif

} //namespace dds
} //namespace fastdds
} //namespace eprosima
