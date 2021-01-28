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

const std::string ThroughputDataType::type_name_ = "ThroughputType";

#include <cstring>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

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
        void* data,
        SerializedPayload_t* payload)
{
    ThroughputType* lt = (ThroughputType*)data;
    memcpy(payload->data, &lt->seqnum, sizeof(lt->seqnum));
    memcpy(payload->data + 4, lt->data, buffer_size_);
    payload->length = 4 + buffer_size_;
    return true;
}

bool ThroughputDataType::deserialize(
        SerializedPayload_t* payload,
        void* data)
{
    if (payload->length > 0)
    {
        // payload members endiannes matches local machine
        ThroughputType* lt = (ThroughputType*)data;
        lt->seqnum = *reinterpret_cast<uint32_t*>(payload->data);
        std::copy(payload->data + 4, payload->data + 4 + buffer_size_, lt->data);
    }
    return true;
}

std::function<uint32_t()> ThroughputDataType::getSerializedSizeProvider(
        void* )
{
    // uint32_t seqnum + uint32_t buffer_size_ + actual data
    uint32_t size = m_typeSize;
    return [size]() -> uint32_t
           {
               return size;
           };
}

void* ThroughputDataType::createData()
{
    return (void*)new uint8_t[(4 + buffer_size_ + 3) / 4 * 4];
}

void ThroughputDataType::deleteData(
        void* data)
{
    delete[] (uint8_t*)(data);
}

bool ThroughputCommandDataType::serialize(
        void* data,
        SerializedPayload_t* p)
{
    ThroughputCommandType* t = (ThroughputCommandType*)data;
    p->length = 0;
    const uint32_t command = t->m_command;
    memcpy(p->data, &command, sizeof(command));
    p->length += sizeof(command);
    memcpy(&p->data[p->length], &t->m_size, sizeof(t->m_size));
    p->length += sizeof(t->m_size);
    memcpy(&p->data[p->length], &t->m_demand, sizeof(t->m_demand));
    p->length += sizeof(t->m_demand);
    memcpy(&p->data[p->length], &t->m_lostsamples, sizeof(t->m_lostsamples));
    p->length += sizeof(t->m_lostsamples);
    memcpy(&p->data[p->length], &t->m_lastrecsample, sizeof(t->m_lastrecsample));
    p->length += sizeof(t->m_lastrecsample);
    memcpy(&p->data[p->length], &t->m_totaltime, sizeof(t->m_totaltime));
    p->length += sizeof(t->m_totaltime);
    return true;
}

bool ThroughputCommandDataType::deserialize(
        SerializedPayload_t* p,
        void* data)
{
    ThroughputCommandType* t = (ThroughputCommandType*)data;
    p->pos = 0;
    uint32_t command;
    memcpy(&command, p->data, sizeof(command));
    t->m_command = static_cast<e_Command>(command);
    p->pos += sizeof(command);
    memcpy(&t->m_size, &p->data[p->pos], sizeof(t->m_size));
    p->pos += sizeof(t->m_size);
    memcpy(&t->m_demand, &p->data[p->pos], sizeof(t->m_demand));
    p->pos += sizeof(t->m_demand);
    memcpy(&t->m_lostsamples, &p->data[p->pos], sizeof(t->m_lostsamples));
    p->pos += sizeof(t->m_lostsamples);
    memcpy(&t->m_lastrecsample, &p->data[p->pos], sizeof(t->m_lastrecsample));
    p->pos += sizeof(t->m_lastrecsample);
    memcpy(&t->m_totaltime, &p->data[p->pos], sizeof(t->m_totaltime));
    p->pos += sizeof(t->m_totaltime);
    return true;
}

std::function<uint32_t()> ThroughputCommandDataType::getSerializedSizeProvider(
        void*)
{
    return []() -> uint32_t
           {
               uint32_t size = 0;

               size = (uint32_t)(sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)  + sizeof(uint32_t) +
                       sizeof(uint64_t) + sizeof(uint64_t));

               return size;
           };
}

void* ThroughputCommandDataType::createData()
{
    return (void*)new ThroughputCommandType();
}

void ThroughputCommandDataType::deleteData(
        void* data)
{
    delete((ThroughputCommandType*)data);
}
