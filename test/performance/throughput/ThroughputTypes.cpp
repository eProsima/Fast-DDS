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
 * @file ThroughputTypes.cpp
 *
 */

#include "ThroughputTypes.hpp"

#include <cstring>
#include <cstddef>

const size_t ThroughputType::overhead = offsetof(ThroughputType, data);
const std::string ThroughputDataType::type_name_ = "ThroughputType";

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

bool ThroughputDataType::compare_data(
        const ThroughputType& lt1,
        const ThroughputType& lt2) const
{
    if (lt1.seqnum != lt2.seqnum)
    {
        return false;
    }

    return 0 == memcmp(lt1.data, lt2.data, buffer_size_);
}

// Serialization and deserialization functions
bool ThroughputDataType::serialize(
        const void* const data,
        SerializedPayload_t& payload,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    static uint8_t encapsulation[4] = { 0x0, 0x1, 0x0, 0x0 };
    ThroughputType* lt = (ThroughputType*)data;

    auto ser_data = payload.data;
    memcpy(ser_data, encapsulation, SerializedPayload_t::representation_header_size);
    ser_data += SerializedPayload_t::representation_header_size;
    memcpy(ser_data, &lt->seqnum, sizeof(lt->seqnum));
    ser_data += sizeof(lt->seqnum);
    memcpy(ser_data, lt->data, buffer_size_);
    payload.length = max_serialized_type_size;
    return true;
}

bool ThroughputDataType::deserialize(
        SerializedPayload_t& payload,
        void* data)
{
    if (payload.length > 0)
    {
        // payload members endiannes matches local machine
        ThroughputType* lt = (ThroughputType*)data;
        auto ser_data = payload.data + SerializedPayload_t::representation_header_size;
        lt->seqnum = *reinterpret_cast<uint32_t*>(ser_data);
        ser_data += sizeof(lt->seqnum);
        std::copy(ser_data, ser_data + buffer_size_, lt->data);
    }
    return true;
}

uint32_t ThroughputDataType::calculate_serialized_size(
        const void* const,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    // uint32_t seqnum + uint32_t buffer_size_ + actual data
    return max_serialized_type_size;
}

void* ThroughputDataType::create_data()
{
    return (void*)new uint8_t[max_serialized_type_size];
}

void ThroughputDataType::delete_data(
        void* data)
{
    delete[] (uint8_t*)(data);
}

bool ThroughputCommandDataType::serialize(
        const void* const data,
        SerializedPayload_t& p,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    ThroughputCommandType* t = (ThroughputCommandType*)data;
    p.length = 0;
    const uint32_t command = t->m_command;
    memcpy(p.data, &command, sizeof(command));
    p.length += sizeof(command);
    memcpy(&p.data[p.length], &t->m_size, sizeof(t->m_size));
    p.length += sizeof(t->m_size);
    memcpy(&p.data[p.length], &t->m_demand, sizeof(t->m_demand));
    p.length += sizeof(t->m_demand);
    memcpy(&p.data[p.length], &t->m_lostsamples, sizeof(t->m_lostsamples));
    p.length += sizeof(t->m_lostsamples);
    memcpy(&p.data[p.length], &t->m_receivedsamples, sizeof(t->m_receivedsamples));
    p.length += sizeof(t->m_receivedsamples);
    memcpy(&p.data[p.length], &t->m_lastrecsample, sizeof(t->m_lastrecsample));
    p.length += sizeof(t->m_lastrecsample);
    memcpy(&p.data[p.length], &t->m_totaltime, sizeof(t->m_totaltime));
    p.length += sizeof(t->m_totaltime);
    return true;
}

bool ThroughputCommandDataType::deserialize(
        SerializedPayload_t& p,
        void* data)
{
    ThroughputCommandType* t = (ThroughputCommandType*)data;
    p.pos = 0;
    uint32_t command;
    memcpy(&command, p.data, sizeof(command));
    t->m_command = static_cast<e_Command>(command);
    p.pos += sizeof(command);
    memcpy(&t->m_size, &p.data[p.pos], sizeof(t->m_size));
    p.pos += sizeof(t->m_size);
    memcpy(&t->m_demand, &p.data[p.pos], sizeof(t->m_demand));
    p.pos += sizeof(t->m_demand);
    memcpy(&t->m_lostsamples, &p.data[p.pos], sizeof(t->m_lostsamples));
    p.pos += sizeof(t->m_lostsamples);
    memcpy(&t->m_receivedsamples, &p.data[p.pos], sizeof(t->m_receivedsamples));
    p.pos += sizeof(t->m_receivedsamples);
    memcpy(&t->m_lastrecsample, &p.data[p.pos], sizeof(t->m_lastrecsample));
    p.pos += sizeof(t->m_lastrecsample);
    memcpy(&t->m_totaltime, &p.data[p.pos], sizeof(t->m_totaltime));
    p.pos += sizeof(t->m_totaltime);
    return true;
}

uint32_t ThroughputCommandDataType::calculate_serialized_size(
        const void* const,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    uint32_t size = 0;

    size = (uint32_t)(sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)  + sizeof(uint32_t) +
            sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t));

    return size;
}

void* ThroughputCommandDataType::create_data()
{
    return (void*)new ThroughputCommandType();
}

void ThroughputCommandDataType::delete_data(
        void* data)
{
    delete((ThroughputCommandType*)data);
}
