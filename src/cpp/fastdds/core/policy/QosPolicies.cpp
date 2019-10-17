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
#include <fastrtps/log/Log.h>
#include <fastcdr/Cdr.h>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace eprosima::fastrtps::rtps;

bool DurabilityQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg, kind);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    return valid;
}

bool DeadlineQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg, period.seconds);
    valid &= CDRMessage::addUInt32(msg, period.fraction());
    return valid;
}

bool LatencyBudgetQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg, duration.seconds);
    valid &= CDRMessage::addUInt32(msg, duration.fraction());
    return valid;
}

bool LivelinessQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg, kind);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addInt32(msg, lease_duration.seconds);
    valid &= CDRMessage::addUInt32(msg, lease_duration.fraction());
    return valid;
}

bool OwnershipQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg, kind);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    return valid;
}

bool ReliabilityQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg, kind);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addInt32(msg, max_blocking_time.seconds);
    valid &= CDRMessage::addUInt32(msg, max_blocking_time.fraction());
    return valid;
}

bool DestinationOrderQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg, kind);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    return valid;
}

bool TimeBasedFilterQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg, minimum_separation.seconds);
    valid &= CDRMessage::addUInt32(msg, minimum_separation.fraction());
    return valid;
}

bool PresentationQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, PARAMETER_PRESENTATION_LENGTH);//this->length);
    valid &= CDRMessage::addOctet(msg, access_scope);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);

    valid &= CDRMessage::addOctet(msg, (octet)coherent_access);
    valid &= CDRMessage::addOctet(msg, (octet)ordered_access);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);

    return valid;
}

bool PartitionQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    //Obtain Length:
    this->length = 0;
    this->length += 4;
    uint16_t rest;
    for (std::vector<std::string>::iterator it = names_.begin(); it != names_.end(); ++it)
    {
        this->length += 4;
        this->length += (uint16_t)it->size() + 1;
        rest = ((uint16_t)it->size() + 1 ) % 4;
        this->length += rest != 0 ? 4 - rest : 0;
    }
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg, (uint32_t)this->names_.size());
    for (std::vector<std::string>::iterator it = names_.begin(); it != names_.end(); ++it)
    {
        valid &= CDRMessage::addString(msg, *it);
    }
    //valid &= CDRMessage::addOctetVector(msg,&name);
    return valid;
}

bool UserDataQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    uint32_t align = (4 - (msg->pos + 6 + data_vec_.size())  % 4) & 3; //align
    this->length = (uint16_t)(4 + this->data_vec_.size() + align);
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg, (uint32_t)this->data_vec_.size());
    valid &= CDRMessage::addData(msg, this->data_vec_.data(), (uint32_t)this->data_vec_.size());
    for (uint32_t count = 0; count < align; ++count)
    {
        valid &= CDRMessage::addOctet(msg, 0);
    }
    return valid;
}

bool TopicDataQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctetVector(msg, &value);
    return valid;
}

bool GroupDataQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctetVector(msg, &value);
    return valid;
}

bool HistoryQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addOctet(msg, kind);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addInt32(msg, depth);
    return valid;
}

bool DurabilityServiceQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg, service_cleanup_delay.seconds);
    valid &= CDRMessage::addUInt32(msg, service_cleanup_delay.fraction());
    valid &= CDRMessage::addOctet(msg, history_kind);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addOctet(msg, 0);
    valid &= CDRMessage::addInt32(msg, history_depth);
    valid &= CDRMessage::addInt32(msg, max_samples);
    valid &= CDRMessage::addInt32(msg, max_instances);
    valid &= CDRMessage::addInt32(msg, max_samples_per_instance);
    return valid;
}

bool LifespanQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addInt32(msg, duration.seconds);
    valid &= CDRMessage::addUInt32(msg, duration.fraction());
    return valid;
}

bool OwnershipStrengthQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addUInt32(msg, value);
    return valid;
}

bool ResourceLimitsQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);

    valid &= CDRMessage::addInt32(msg, max_samples);
    valid &= CDRMessage::addInt32(msg, max_instances);
    valid &= CDRMessage::addInt32(msg, max_samples_per_instance);
    return valid;
}

bool TransportPriorityQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);//this->length);
    valid &= CDRMessage::addUInt32(msg, value);
    return valid;
}

bool DataRepresentationQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    this->length = 4;
    this->length += static_cast<uint16_t>(m_value.size() * sizeof(uint16_t));
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt32(msg, static_cast<uint32_t>(m_value.size()));
    for (DataRepresentationId_t id : m_value)
    {
        valid &= CDRMessage::addUInt16(msg, static_cast<uint16_t>(id));
    }
    if (m_value.size() % 2 == 1) // Odd, we must align
    {
        valid &= CDRMessage::addUInt16(msg, uint16_t(0));
    }
    return valid;
}

bool TypeConsistencyEnforcementQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
{
    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    valid &= CDRMessage::addUInt16(msg, this->length);
    valid &= CDRMessage::addUInt16(msg, this->m_kind);
    valid &= CDRMessage::addOctet(msg, static_cast<octet>(m_ignore_sequence_bounds));
    valid &= CDRMessage::addOctet(msg, static_cast<octet>(m_ignore_string_bounds));
    valid &= CDRMessage::addOctet(msg, static_cast<octet>(m_ignore_member_names));
    valid &= CDRMessage::addOctet(msg, static_cast<octet>(m_prevent_type_widening));
    valid &= CDRMessage::addOctet(msg, static_cast<octet>(m_force_type_validation));
    valid &= CDRMessage::addOctet(msg, octet(0x00)); // 8th byte
    return valid;
}

bool DisablePositiveACKsQosPolicy::addToCDRMessage(
        CDRMessage_t* msg)
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

bool TypeIdV1::addToCDRMessage(
        CDRMessage_t* msg)
{
    size_t size = fastrtps::types::TypeIdentifier::getCdrSerializedSize(m_type_identifier) + 4;
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
    valid &= CDRMessage::addData(msg, payload.data, payload.length);

    uint32_t align = 4 - (payload.length % 4); //align
    for (uint32_t count = 0; count < align; ++count)
    {
        valid &= CDRMessage::addOctet(msg, 0);
    }

    return valid;
}

bool TypeIdV1::readFromCDRMessage(
        CDRMessage_t* msg,
        uint32_t size)
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
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

bool TypeObjectV1::addToCDRMessage(
        CDRMessage_t* msg)
{
    size_t size = fastrtps::types::TypeObject::getCdrSerializedSize(m_type_object) + 4;
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

bool TypeObjectV1::readFromCDRMessage(
        CDRMessage_t* msg,
        uint32_t size)
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
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

bool xtypes::TypeInformation::addToCDRMessage(
        CDRMessage_t* msg)
{
    size_t size = fastrtps::types::TypeInformation::getCdrSerializedSize(type_information) + 4;
    SerializedPayload_t payload(static_cast<uint32_t>(size));
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload.data, payload.max_size);

    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that serializes the data.
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    ser.serialize_encapsulation();

    type_information.serialize(ser);
    payload.length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length

    bool valid = CDRMessage::addUInt16(msg, this->Pid);
    this->length = static_cast<uint16_t>(payload.length);
    valid &= CDRMessage::addUInt16(msg, this->length);

    return valid & CDRMessage::addData(msg, payload.data, payload.length);
}

bool xtypes::TypeInformation::readFromCDRMessage(
        CDRMessage_t* msg,
        uint32_t size)
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
        type_information.deserialize(deser);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    this->assigned_ = true;

    return true;
}

} //namespace dds
} //namespace fastdds
} //namespace eprosima
