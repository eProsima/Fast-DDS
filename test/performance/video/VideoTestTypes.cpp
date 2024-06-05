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
 * @file VideoTestTypes.cpp
 *
 */

#include "VideoTestTypes.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

bool VideoDataType::serialize(
        void* data,
        eprosima::fastdds::rtps::SerializedPayload_t* payload)
{
    VideoType* lt = (VideoType*)data;


    *(uint32_t*)payload->data = lt->seqnum;
    *(uint64_t*)(payload->data + 4) = lt->timestamp;
    *(uint64_t*)(payload->data + 12) = lt->duration;
    *(uint32_t*)(payload->data + 20) = (uint32_t)lt->data.size();
    memcpy(payload->data + 24, lt->data.data(), lt->data.size());

    payload->length = (uint32_t)(24 + lt->data.size());
    return true;
}

bool VideoDataType::deserialize(
        eprosima::fastdds::rtps::SerializedPayload_t* payload,
        void* data)
{
    VideoType* lt = (VideoType*)data;
    lt->seqnum = *(uint32_t*)payload->data;
    lt->timestamp = *(uint64_t*)(payload->data + 4);
    lt->duration = *(uint64_t*)(payload->data + 12);
    uint32_t siz = *(uint32_t*)(payload->data + 20);
    lt->data.resize(siz + 1);
    std::copy(payload->data + 24, payload->data + 24 + siz, lt->data.begin());
    return true;
}

std::function<uint32_t()> VideoDataType::getSerializedSizeProvider(
        void* data)
{
    return [data]() -> uint32_t
           {
               VideoType* tdata = static_cast<VideoType*>(data);
               uint32_t size = 0;

               size =
                       (uint32_t)(sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t) +
                       tdata->data.size());

               return size;
           };
}

void* VideoDataType::createData()
{

    return (void*)new VideoType();
}

void VideoDataType::deleteData(
        void* data)
{

    delete((VideoType*)data);
}

bool TestCommandDataType::serialize(
        void* data,
        SerializedPayload_t* payload)
{
    TestCommandType* t = (TestCommandType*)data;
    *(TESTCOMMAND*)payload->data = t->m_command;
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
    t->m_command = *(TESTCOMMAND*)payload->data;
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
