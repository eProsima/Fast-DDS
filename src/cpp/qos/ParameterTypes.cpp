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

#include <fastrtps/qos/ParameterTypes.h>

#include <fastrtps/rtps/messages/CDRMessage.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

// PARAMETER LOCATOR
bool ParameterLocator_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_LOCATOR_LENGTH);//this->length);
    valid &= CDRMessage::addLocator(msg, &this->locator);
    return valid;
}

bool ParameterLocator_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readLocator(msg, &locator);
}

//PARAMTERKEY
bool ParameterKey_t::addToCDRMessage(CDRMessage_t* msg)
{
    return CDRMessage::addParameterKey(msg,&this->key);
}

bool ParameterKey_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readData(msg, key.value, PARAMETER_KEY_LENGTH);
}

// PARAMETER_ STRING
bool ParameterString_t::addToCDRMessage(CDRMessage_t* msg)
{
    if(this->m_string.size()==0)
        return false;
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    //Str size
    uint32_t str_siz = (uint32_t)this->m_string.size() + 1;
    int rest = (str_siz) % 4;
    if (rest != 0)
        rest = 4 - rest; //how many you have to add
    this->length = (uint16_t)(str_siz + 4 + rest);
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::add_string(msg, this->m_string);
    return valid;
}

bool ParameterString_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    string_255 aux;
    bool valid = CDRMessage::readString(msg, &aux);
    setName(aux.c_str());
    return valid;
}

// PARAMETER_ PORT
bool ParameterPort_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PORT_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg, this->port);
    return valid;
}

bool ParameterPort_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readUInt32(msg, &port);
}

//PARAMETER_ GUID
bool ParameterGuid_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_GUID_LENGTH);//this->length);
    valid &= CDRMessage::addData(msg,this->guid.guidPrefix.value,12);
    valid &= CDRMessage::addData(msg,this->guid.entityId.value,4);
    return valid;
}

bool ParameterGuid_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid =  CDRMessage::readData(msg, guid.guidPrefix.value, 12);
    valid &= CDRMessage::readData(msg, guid.entityId.value, 4);
    return valid;
}

//PARAMETER_ PROTOCOL VERSION
bool ParameterProtocolVersion_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PROTOCOL_LENGTH);//this->length);
    valid &= CDRMessage::addOctet(msg,protocolVersion.m_major);
    valid &= CDRMessage::addOctet(msg,protocolVersion.m_minor);
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterProtocolVersion_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, &protocolVersion.m_major);
    valid &= CDRMessage::readOctet(msg, &protocolVersion.m_minor);
    msg->pos += 2; //padding
    return valid;
}

bool ParameterVendorId_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_VENDOR_LENGTH);//this->length);
    valid &= CDRMessage::addOctet(msg,vendorId[0]);
    valid &= CDRMessage::addOctet(msg,vendorId[1]);
    valid &= CDRMessage::addUInt16(msg, 0);
    return valid;
}

bool ParameterVendorId_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, &vendorId[0]);
    valid &= CDRMessage::readOctet(msg, &vendorId[1]);
    msg->pos += 2; //padding
    return valid;
}

//PARAMETER_ IP4ADDRESS
bool ParameterIP4Address_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_IP4_LENGTH);//this->length);
    valid &= CDRMessage::addData(msg,this->address,4);
    return valid;
}

bool ParameterIP4Address_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readData(msg, address, 4);
}

void ParameterIP4Address_t::setIP4Address(octet o1,octet o2,octet o3,octet o4){
    address[0] = o1;
    address[1] = o2;
    address[2] = o3;
    address[3] = o4;
}

bool ParameterBool_t::addToCDRMessage(CDRMessage_t* msg){
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_BOOL_LENGTH);//this->length);
    octet val = value ? 1:0;
    valid &= CDRMessage::addOctet(msg,val);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addUInt16(msg,0);
    return valid;
}

bool ParameterBool_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&value);
    msg->pos += 3; //padding
    return valid;
}

bool ParameterStatusInfo_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_STATUS_INFO_LENGTH);//this->length);
    valid &= CDRMessage::addUInt16(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, status);
    return valid;
}

bool ParameterStatusInfo_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    //octet status = msg.buffer[msg.pos + 3];
    bool valid = true;
    octet tmp;
    //Remove the front three octets, take the fourth
    valid &= CDRMessage::readOctet(msg, &tmp);
    valid &= CDRMessage::readOctet(msg, &tmp);
    valid &= CDRMessage::readOctet(msg, &tmp);
    return CDRMessage::readOctet(msg, &status);
}

bool ParameterCount_t::addToCDRMessage(CDRMessage_t* msg){
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_COUNT_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg,count);
    return valid;
}

bool ParameterCount_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readUInt32(msg, &count);
}

bool ParameterEntityId_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_ENTITYID_LENGTH);//this->length);
    valid &= CDRMessage::addEntityId(msg,&entityId);
    return valid;
}

bool ParameterEntityId_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readEntityId(msg, &entityId);
}

bool ParameterTime_t::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_TIME_LENGTH);//this->length);
    valid &= CDRMessage::addInt32(msg, time.seconds());
    valid &= CDRMessage::addInt32(msg, time.fraction());
    return valid;
}

bool ParameterTime_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    int32_t sec(0);
    bool valid = CDRMessage::readInt32(msg, &sec);
    time.seconds(sec);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    time.fraction(frac);
    return valid;
}

bool ParameterBuiltinEndpointSet_t::addToCDRMessage(CDRMessage_t*msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_BUILTINENDPOINTSET_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg,this->endpointSet);
    return valid;
}

bool ParameterBuiltinEndpointSet_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readUInt32(msg, &endpointSet);
}

bool ParameterPropertyList_t::addToCDRMessage(CDRMessage_t*msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    uint16_t pos_str = (uint16_t)msg->pos;
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addUInt32(msg,(uint32_t)this->size());
    for(ParameterPropertyList_t::iterator it = this->begin();
            it!=this->end();++it)
    {
        //it is a custom iterator with no operator-> overload
        valid &= CDRMessage::add_string(msg,(*it).first());
        valid &= CDRMessage::add_string(msg,(*it).second());
    }
    uint16_t pos_param_end = (uint16_t)msg->pos;
    this->length = pos_param_end-pos_str-2;
    msg->pos = pos_str;
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    msg->pos = pos_param_end;
    msg->length-=2;
    return valid;
}

bool ParameterPropertyList_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    if (limit_size_ && properties_.max_size < size - 4)
    {
        return false;
    }

    uint32_t num_properties;
    bool valid = CDRMessage::readUInt32(msg, &num_properties);

    properties_.reserve(size - 4);

    for(size_t i = 0; i < num_properties; ++i)
    {
        uint32_t property_size, alignment;

        valid &= CDRMessage::readUInt32(msg,&property_size);
        if (!valid)
        {
            return false;
        }
        alignment = ((property_size + 3) & ~3) - property_size;
        push_back_helper (&msg->buffer[msg->pos], property_size, alignment);
        msg->pos += (property_size + alignment);

        valid &= CDRMessage::readUInt32(msg,&property_size);
        if (!valid)
        {
            return false;
        }
        alignment = ((property_size + 3) & ~3) - property_size;
        push_back_helper (&msg->buffer[msg->pos], property_size, alignment);
        msg->pos += (property_size + alignment);
    }
    Nproperties_ = num_properties;
    return valid;
}

bool ParameterSampleIdentity_t::addToCDRMessage(CDRMessage_t*msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addData(msg, sample_id.writer_guid().guidPrefix.value, GuidPrefix_t::size);
    valid &= CDRMessage::addData(msg, sample_id.writer_guid().entityId.value, EntityId_t::size);
    valid &= CDRMessage::addInt32(msg, sample_id.sequence_number().high);
    valid &= CDRMessage::addUInt32(msg, sample_id.sequence_number().low);
    return valid;
}

bool ParameterSampleIdentity_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readData(msg, sample_id.writer_guid().guidPrefix.value, GuidPrefix_t::size);
    valid &= CDRMessage::readData(msg, sample_id.writer_guid().entityId.value, EntityId_t::size);
    valid &= CDRMessage::readInt32(msg, &sample_id.sequence_number().high);
    valid &= CDRMessage::readUInt32(msg, &sample_id.sequence_number().low);
    return valid;
}

#if HAVE_SECURITY

bool ParameterToken_t::addToCDRMessage(CDRMessage_t*msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    uint16_t pos_str = (uint16_t)msg->pos;
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addDataHolder(msg, this->token);
    uint32_t align = (4 - msg->pos % 4) & 3; //align
    for(uint32_t count = 0; count < align; ++count)
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

bool ParameterToken_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readDataHolder(msg, token);
}

bool ParameterParticipantSecurityInfo_t::addToCDRMessage(CDRMessage_t*msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg, this->security_attributes);
    valid &= CDRMessage::addUInt32(msg, this->plugin_security_attributes);
    return valid;
}

bool ParameterParticipantSecurityInfo_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readUInt32(msg, &security_attributes);
    valid &= CDRMessage::readUInt32(msg, &plugin_security_attributes);
    return valid;
}

bool ParameterEndpointSecurityInfo_t::addToCDRMessage(CDRMessage_t*msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH);//this->length);
    valid &= CDRMessage::addUInt32(msg, this->security_attributes);
    valid &= CDRMessage::addUInt32(msg, this->plugin_security_attributes);
    return valid;
}

bool ParameterEndpointSecurityInfo_t::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readUInt32(msg, &security_attributes);
    valid &= CDRMessage::readUInt32(msg, &plugin_security_attributes);
    return valid;
}

#endif
