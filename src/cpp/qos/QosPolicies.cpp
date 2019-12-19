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

#include <fastrtps/qos/QosPolicies.h>

#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/log/Log.h>
#include <fastcdr/Cdr.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

bool DurabilityQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    return valid;
}

bool DurabilityQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&kind);
    msg->pos += 3; //padding
    return valid;
}

bool DeadlineQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,period.seconds);
    valid &= CDRMessage::addUInt32(msg,period.fraction());
    return valid;
}

bool DeadlineQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readInt32(msg, &period.seconds);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    period.fraction(frac);
    return valid;
}

bool LatencyBudgetQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,duration.seconds);
    valid &= CDRMessage::addUInt32(msg,duration.fraction());
    return valid;
}

bool LatencyBudgetQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readInt32(msg, &duration.seconds);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    duration.fraction(frac);
    return valid;
}

bool LivelinessQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addInt32(msg,lease_duration.seconds);
    valid &= CDRMessage::addUInt32(msg,lease_duration.fraction());
    return valid;
}

bool LivelinessQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&kind);
    msg->pos += 3; //padding
    valid &= CDRMessage::readInt32(msg, &lease_duration.seconds);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    lease_duration.fraction(frac);
    return valid;
}

bool OwnershipQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    return valid;
}

bool OwnershipQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&kind);
    msg->pos += 3; //padding
    return valid;
}

bool ReliabilityQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addInt32(msg,max_blocking_time.seconds);
    valid &= CDRMessage::addUInt32(msg,max_blocking_time.fraction());
    return valid;
}

bool ReliabilityQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&kind);
    msg->pos += 3; //padding
    valid &= CDRMessage::readInt32(msg, &max_blocking_time.seconds);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    max_blocking_time.fraction(frac);
    return valid;
}

bool DestinationOrderQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    return valid;
}

bool DestinationOrderQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&kind);
    msg->pos += 3; //padding
    return valid;
}

bool TimeBasedFilterQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,minimum_separation.seconds);
    valid &= CDRMessage::addUInt32(msg,minimum_separation.fraction());
    return valid;
}

bool TimeBasedFilterQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readInt32(msg, &minimum_separation.seconds);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    minimum_separation.fraction(frac);
    return valid;
}

bool PresentationQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PRESENTATION_LENGTH);//this->length);
    valid &= CDRMessage::addOctet(msg,access_scope);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);

    valid &= CDRMessage::addOctet(msg,(octet)coherent_access);
    valid &= CDRMessage::addOctet(msg,(octet)ordered_access);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);

    return valid;
}

bool PresentationQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&access_scope);
    msg->pos += 3; //padding
    valid &= CDRMessage::readOctet(msg, (octet*)&coherent_access);
    valid &= CDRMessage::readOctet(msg, (octet*)&ordered_access);
    msg->pos += 2; //padding
    return valid;
}

bool PartitionQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    //Obtain Length:
    this->length = 0;
    this->length += 4;
    uint16_t rest;
    for(std::vector<std::string>::iterator it = names.begin();it!=names.end();++it)
    {
        this->length +=4;
        this->length += (uint16_t)it->size()+1;
        rest = ((uint16_t)it->size() +1 ) % 4;
        this->length += rest != 0 ? 4 - rest : 0;
    }
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg,(uint32_t)this->names.size());
    for(std::vector<std::string>::iterator it = names.begin();it!=names.end();++it)
        valid &= CDRMessage::addString(msg,*it);
    //valid &= CDRMessage::addOctetVector(msg,&name);
    return valid;
}

bool PartitionQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    uint32_t namessize = 0;
    bool valid = CDRMessage::readUInt32(msg, &namessize);
    if (!valid)
    {
        return false;
    }
    for (uint32_t i = 1; i <= namessize; ++i)
    {
        std::string auxstr;
        valid &= CDRMessage::readString(msg, &auxstr);
        if (!valid)
        {
            return false;
        }
        names.push_back(auxstr);
    }

    return valid;
}

bool UserDataQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    uint32_t align = (4 - (msg->pos + 6 + dataVec_.size())  % 4) & 3; //align
    this->length = (uint16_t)(4 + this->dataVec_.size() + align);
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg, (uint32_t)this->dataVec_.size());
    valid &= CDRMessage::addData(msg,this->dataVec_.data(),(uint32_t)this->dataVec_.size());
    for(uint32_t count = 0; count < align; ++count)
    {
        valid &= CDRMessage::addOctet(msg, 0);
    }
    return valid;
}

bool UserDataQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    if (max_size_ != 0 && size > max_size_)
    {
        return false;
    }
    return CDRMessage::readOctetVector(msg, &dataVec_);
}

bool TopicDataQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctetVector(msg,&value);
    return valid;
}

bool TopicDataQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readOctetVector(msg, &value);
}

bool GroupDataQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctetVector(msg,&value);
    return valid;
}

bool GroupDataQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readOctetVector(msg, &value);
}

bool HistoryQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg,kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addInt32(msg,depth);
    return valid;
}

bool HistoryQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readOctet(msg, (octet*)&kind);
    msg->pos += 3; //padding
    valid &= CDRMessage::readInt32(msg, &depth);
    return valid;
}

bool DurabilityServiceQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,service_cleanup_delay.seconds);
    valid &= CDRMessage::addUInt32(msg,service_cleanup_delay.fraction());
    valid &= CDRMessage::addOctet(msg,history_kind);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addOctet(msg,0);
    valid &= CDRMessage::addInt32(msg,history_depth);
    valid &= CDRMessage::addInt32(msg,max_samples);
    valid &= CDRMessage::addInt32(msg,max_instances);
    valid &= CDRMessage::addInt32(msg,max_samples_per_instance);
    return valid;
}

bool DurabilityServiceQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readInt32(msg, &service_cleanup_delay.seconds);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    service_cleanup_delay.fraction(frac);
    valid &= CDRMessage::readOctet(msg, (octet*)&history_kind);
    msg->pos += 3; //padding
    valid &= CDRMessage::readInt32(msg, &history_depth);
    valid &= CDRMessage::readInt32(msg, &max_samples);
    valid &= CDRMessage::readInt32(msg, &max_instances);
    valid &= CDRMessage::readInt32(msg, &max_samples_per_instance);
    return valid;
}

bool LifespanQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg,duration.seconds);
    valid &= CDRMessage::addUInt32(msg,duration.fraction());
    return valid;
}

bool LifespanQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readInt32(msg, &duration.seconds);
    uint32_t frac(0);
    valid &= CDRMessage::readUInt32(msg, &frac);
    duration.fraction(frac);
    return valid;
}

bool OwnershipStrengthQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addUInt32(msg,value);
    return valid;
}

bool OwnershipStrengthQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readUInt32(msg, &value);
}

bool ResourceLimitsQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);

    valid &= CDRMessage::addInt32(msg,max_samples);
    valid &= CDRMessage::addInt32(msg,max_instances);
    valid &= CDRMessage::addInt32(msg,max_samples_per_instance);
    return valid;
}

bool ResourceLimitsQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    bool valid = CDRMessage::readInt32(msg, &max_samples);
    valid &= CDRMessage::readInt32(msg, &max_instances);
    valid &= CDRMessage::readInt32(msg, &max_samples_per_instance);
    return valid;
}

bool TransportPriorityQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addUInt32(msg, value);
    return valid;
}

bool TransportPriorityQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    return CDRMessage::readUInt32(msg, &value);
}

bool DataRepresentationQosPolicy::addToCDRMessage(CDRMessage_t* msg) {
    bool valid = CDRMessage::addUInt32(msg, (uint32_t)m_value.size());
    for (std::vector<DataRepresentationId_t>::iterator it = m_value.begin(); it != m_value.end(); ++it)
        valid &= CDRMessage::addUInt16(msg, *it);
    return valid;
}

bool DataRepresentationQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    int16_t temp(0);
    uint32_t datasize(0);
    bool valid = CDRMessage::readUInt32(msg, &datasize);
    for (uint32_t i = 0; i < datasize; ++i)
    {
        valid &= CDRMessage::readInt16(msg, &temp);
        m_value.push_back(static_cast<DataRepresentationId_t>(temp));
    }
    return valid;

}

bool TypeConsistencyEnforcementQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt16(msg, this->m_kind);
    valid &= CDRMessage::addOctet(msg, (octet)m_ignore_sequence_bounds);
    valid &= CDRMessage::addOctet(msg, (octet)m_ignore_string_bounds);
    valid &= CDRMessage::addOctet(msg, (octet)m_ignore_member_names);
    valid &= CDRMessage::addOctet(msg, (octet)m_prevent_type_widening);
    valid &= CDRMessage::addOctet(msg, (octet)m_force_type_validation);
    valid &= CDRMessage::addOctet(msg, octet(0x00)); // 8th byte
    return valid;
}

bool TypeConsistencyEnforcementQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    uint16_t uKind(0);
    octet temp(0);
    m_ignore_sequence_bounds = false;
    m_ignore_string_bounds = false;
    m_ignore_member_names = false;
    m_prevent_type_widening = false;
    m_force_type_validation = false;

    bool valid = CDRMessage::readUInt16(msg, &uKind);
    m_kind = static_cast<TypeConsistencyKind>(uKind);

    if (size >= 3)
    {
        valid &= CDRMessage::readOctet(msg, &temp);
        m_ignore_sequence_bounds = temp == 0 ? false : true;
    }

    if (valid && size >= 4)
    {
        valid &= CDRMessage::readOctet(msg, &temp);
        m_ignore_string_bounds = temp == 0 ? false : true;
    }

    if (valid && size >= 5)
    {
        valid &= CDRMessage::readOctet(msg, &temp);
        m_ignore_member_names = temp == 0 ? false : true;
    }

    if (valid && size >= 6)
    {
        valid &= CDRMessage::readOctet(msg, &temp);
        m_prevent_type_widening = temp == 0 ? false : true;
    }

    if (valid && size >= 7)
    {
        valid &= CDRMessage::readOctet(msg, &temp);
        m_force_type_validation = temp == 0 ? false : true;
    }
    return valid;
}

bool DisablePositiveACKsQosPolicy::addToCDRMessage(CDRMessage_t* msg)
{
    if (enabled)
    {
        bool valid = CDRMessage::addUInt16(msg, this->Pid);
        valid &= CDRMessage::addUInt16(msg, this->length);
        valid &= CDRMessage::addOctet(msg, (octet)0x01);
        valid &= CDRMessage::addOctet(msg, (octet)0x00);
        valid &= CDRMessage::addOctet(msg, (octet)0x00);
        valid &= CDRMessage::addOctet(msg, (octet)0x00);
        return valid;
    }

    return true;
}

bool DisablePositiveACKsQosPolicy::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    (void) size;
    octet value(0);
    bool valid = CDRMessage::readOctet(msg, &value);
    enabled = (value == 0) ? false : true;
    msg->pos += 3; //padding
    return valid;
}

bool TypeIdV1::addToCDRMessage(CDRMessage_t* msg)
{
    size_t size = types::TypeIdentifier::getCdrSerializedSize(m_type_identifier) + 4;
    SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.serialize_encapsulation();

    m_type_identifier.serialize(ser);
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length

    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    this->length = static_cast<uint16_t>(payload.length);
    valid &= CDRMessage::addUInt16(msg, this->length);

    return valid & CDRMessage::addData(msg, payload.data, payload.length);
}

bool TypeIdV1::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    SerializedPayload_t payload(size);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload.data, size);

    CDRMessage::readData(msg, payload.data, size); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.

    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        m_type_identifier.deserialize(deser);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

bool TypeObjectV1::addToCDRMessage(CDRMessage_t* msg)
{
    size_t size = types::TypeObject::getCdrSerializedSize(m_type_object) + 4;
    SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.serialize_encapsulation();

    m_type_object.serialize(ser);
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length

    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    this->length = static_cast<uint16_t>(payload.length);
    valid &= CDRMessage::addUInt16(msg, this->length);

    return valid & CDRMessage::addData(msg, payload.data, payload.length);
}

bool TypeObjectV1::readFromCDRMessage(CDRMessage_t* msg, uint32_t size)
{
    SerializedPayload_t payload(size);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload.data, size);

    CDRMessage::readData(msg, payload.data, size); // Object that manages the raw buffer.

    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.

    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        m_type_object.deserialize(deser);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}
