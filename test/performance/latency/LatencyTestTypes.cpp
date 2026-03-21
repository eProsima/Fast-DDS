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
 * @file LatencyTestTypes.cpp
 *
 */

#include "LatencyTestTypes.hpp"

#include <cstring>
#include <cstddef>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

const size_t LatencyType::overhead = offsetof(LatencyType, data) +
        SerializedPayload_t::representation_header_size;
const std::string LatencyDataType::type_name_ = "LatencyType";

bool LatencyDataType::compare_data(
        const LatencyType& lt1,
        const LatencyType& lt2) const
{
    if (lt1.seqnum != lt2.seqnum)
    {
        return false;
    }

    // bouncing time is ignored on comparison

    return 0 == memcmp(lt1.data, lt2.data, buffer_size_);
}

void LatencyDataType::copy_data(
        const LatencyType& src,
        LatencyType& dst) const
{

    dst.seqnum = src.seqnum;
    dst.bounce = src.bounce;
    memcpy(dst.data, src.data, buffer_size_);
}

bool LatencyDataType::serialize(
        const void* const data,
        SerializedPayload_t& payload,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    static uint8_t encapsulation[4] = { 0x0, 0x1, 0x0, 0x0 };
    LatencyType* lt = (LatencyType*)data;

    auto ser_data = payload.data;
    memcpy(ser_data, encapsulation, SerializedPayload_t::representation_header_size);
    ser_data += SerializedPayload_t::representation_header_size;
    memcpy(ser_data, &lt->seqnum, sizeof(lt->seqnum));
    ser_data += sizeof(lt->seqnum);
    memcpy(ser_data, &lt->bounce, sizeof(lt->bounce));
    ser_data += sizeof(lt->bounce);
    memcpy(ser_data, lt->data, buffer_size_);
    payload.length = max_serialized_type_size;
    return true;
}

bool LatencyDataType::deserialize(
        SerializedPayload_t& payload,
        void* data)
{
    // Payload members endianness matches local machine
    LatencyType* lt = (LatencyType*)data;
    auto ser_data = payload.data + SerializedPayload_t::representation_header_size;
    lt->seqnum = *reinterpret_cast<uint32_t*>(ser_data);
    ser_data += sizeof(lt->seqnum);
    lt->bounce = *reinterpret_cast<uint32_t*>(ser_data);
    ser_data += sizeof(lt->bounce);
    std::copy(ser_data, ser_data + buffer_size_, lt->data);
    return true;
}

uint32_t LatencyDataType::calculate_serialized_size(
        const void* const,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    return max_serialized_type_size;
}

void* LatencyDataType::create_data()
{
    return (void*)new uint8_t[max_serialized_type_size];
}

void LatencyDataType::delete_data(
        void* data)
{
    delete[] (uint8_t*)(data);
}

bool TestCommandDataType::serialize(
        const void* const data,
        SerializedPayload_t& payload,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    TestCommandType* t = (TestCommandType*)data;
    memcpy(payload.data, &t->m_command, sizeof(t->m_command));
    payload.length = 4;
    return true;
}

bool TestCommandDataType::deserialize(
        SerializedPayload_t& payload,
        void* data)
{
    TestCommandType* t = (TestCommandType*)data;
    //	cout << "PAYLOAD LENGTH: "<<payload->length << endl;
    //	cout << "PAYLOAD FIRST BYTE: "<< (int)payload->data[0] << endl;
    memcpy(&t->m_command, payload.data, sizeof(payload.length));
    //	cout << "COMMAND: "<<t->m_command<< endl;
    return true;
}

uint32_t TestCommandDataType::calculate_serialized_size(
        const void* const,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    uint32_t size = 0;

    size = (uint32_t)sizeof(uint32_t);

    return size;
}

void* TestCommandDataType::create_data()
{

    return (void*)new TestCommandType();
}

void TestCommandDataType::delete_data(
        void* data)
{

    delete((TestCommandType*)data);
}
