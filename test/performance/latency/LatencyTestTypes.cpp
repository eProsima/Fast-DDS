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

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

const std::string LatencyDataType::type_name_ = "LatencyType";

bool LatencyDataType::compare_data(
        const LatencyType& lt1,
        const LatencyType& lt2) const
{
    if (lt1.seqnum != lt2.seqnum)
    {
        return false;
    }

    return 0 == memcmp(lt1.data, lt2.data, buffer_size_);
}

bool LatencyDataType::serialize(
        void* data,
        SerializedPayload_t* payload)
{
    LatencyType* lt = (LatencyType*)data;

    memcpy(payload->data, &lt->seqnum, sizeof(lt->seqnum));
    memcpy(payload->data + 4, lt->data, buffer_size_);
    payload->length = 4 + buffer_size_;
    return true;
}

bool LatencyDataType::deserialize(
        SerializedPayload_t* payload,
        void* data)
{
    // Payload members endianness matches local machine
    LatencyType* lt = (LatencyType*)data;
    lt->seqnum = *reinterpret_cast<uint32_t*>(payload->data);
    std::copy(payload->data + 4, payload->data + 4 + buffer_size_, lt->data);
    return true;
}

std::function<uint32_t()> LatencyDataType::getSerializedSizeProvider(
        void*)
{
    uint32_t size = m_typeSize;
    return [size]() -> uint32_t
           {
               return size;
           };
}

void* LatencyDataType::createData()
{
    return (void*)new uint8_t[m_typeSize];
}

void LatencyDataType::deleteData(
        void* data)
{
    delete[] (uint8_t*)(data);
}

bool TestCommandDataType::serialize(
        void* data,
        SerializedPayload_t* payload)
{
    TestCommandType* t = (TestCommandType*)data;
    memcpy(payload->data, &t->m_command, sizeof(t->m_command));
    payload->length = 4;
    return true;
}

bool TestCommandDataType::deserialize(
        SerializedPayload_t* payload,
        void* data)
{
    TestCommandType* t = (TestCommandType*)data;
    //	cout << "PAYLOAD LENGTH: "<<payload->length << endl;
    //	cout << "PAYLOAD FIRST BYTE: "<< (int)payload->data[0] << endl;
    memcpy(&t->m_command, payload->data, sizeof(payload->length));
    //	cout << "COMMAND: "<<t->m_command<< endl;
    return true;
}

std::function<uint32_t()> TestCommandDataType::getSerializedSizeProvider(
        void*)
{
    return []() -> uint32_t
           {
               uint32_t size = 0;

               size = (uint32_t)sizeof(uint32_t);

               return size;
           };
}

void* TestCommandDataType::createData()
{

    return (void*)new TestCommandType();
}

void TestCommandDataType::deleteData(
        void* data)
{

    delete((TestCommandType*)data);
}
