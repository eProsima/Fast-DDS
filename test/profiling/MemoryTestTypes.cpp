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
 * @file MemoryTestTypes.cpp
 *
 */

#include "MemoryTestTypes.h"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

bool MemoryDataType::serialize(
        const void* const data,
        SerializedPayload_t& payload,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    const MemoryType* lt = static_cast<const MemoryType*>(data);


    *(uint32_t*)payload.data = lt->seqnum;
    *(uint32_t*)(payload.data + 4) = (uint32_t)lt->data.size();

    memcpy(payload.data + 8, lt->data.data(), lt->data.size());
    payload.length = static_cast<uint32_t>(8 + lt->data.size());
    return true;
}

bool MemoryDataType::deserialize(
        SerializedPayload_t& payload,
        void* data)
{
    MemoryType* lt = static_cast<MemoryType*>(data);
    lt->seqnum = static_cast<uint32_t>(*(payload.data));
    uint32_t siz = static_cast<uint32_t>(*(payload.data + 4));
    std::copy(payload.data + 8, payload.data + 8 + siz, lt->data.begin());
    return true;
}

uint32_t MemoryDataType::calculate_serialized_size(
        const void* const data,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    const MemoryType* tdata = static_cast<const MemoryType*>(data);
    uint32_t size = 0;

    size = static_cast<uint32_t>(sizeof(uint32_t) + sizeof(uint32_t) + tdata->data.size());

    return size;
}

void* MemoryDataType::create_data()
{

    return static_cast<void*>(new MemoryType());
}

void MemoryDataType::delete_data(
        void* data)
{

    delete(static_cast<MemoryType*>(data));
}

bool TestCommandDataType::serialize(
        const void* const data,
        SerializedPayload_t& payload,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    const TestCommandType* t = static_cast<const TestCommandType*>(data);
    *(TESTCOMMAND*)payload.data = t->m_command;
    payload.length = 4;
    return true;
}

bool TestCommandDataType::deserialize(
        SerializedPayload_t& payload,
        void* data)
{
    TestCommandType* t = static_cast<TestCommandType*>(data);
    t->m_command = static_cast<TESTCOMMAND>(*(payload.data));
    return true;
}

uint32_t TestCommandDataType::calculate_serialized_size(
        const void* const,
        eprosima::fastdds::dds::DataRepresentationId_t)
{
    uint32_t size = 0;

    size = static_cast<uint32_t>(sizeof(uint32_t));

    return size;
}

void* TestCommandDataType::create_data()
{

    return static_cast<void*>(new TestCommandType());
}

void TestCommandDataType::delete_data(
        void* data)
{

    delete(static_cast<TestCommandType*>(data));
}
