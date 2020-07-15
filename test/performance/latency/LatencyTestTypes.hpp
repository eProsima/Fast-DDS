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
 * @file LatencyTestTypes.h
 *
 */

#include <stdint.h>
#include <vector>

#ifndef LATENCYTESTTYPES_H_
#define LATENCYTESTTYPES_H_

#include "fastrtps/fastrtps_all.h"

#define MAX_TYPE_SIZE 10 * 1024 * 1024

class LatencyDataSizes
{
public:

    LatencyDataSizes()
    {
        sample_sizes_ = {16 - 4, 1024 - 4, 64512 - 4, 1048576 - 4};
    }

    inline std::vector<uint32_t>& sample_sizes()
    {
        return sample_sizes_;
    }

private:

    std::vector<uint32_t> sample_sizes_;

};

class LatencyType
{
public:

    uint32_t seqnum;
    std::vector<uint8_t> data;

    LatencyType()
        : seqnum(0)
    {
    }

    LatencyType(
            uint32_t number)
        : seqnum(0)
        , data(number, 0)
    {
    }

    ~LatencyType()
    {
    }

};

inline bool operator ==(
        const LatencyType& lt1,
        const LatencyType& lt2)
{
    if (lt1.seqnum != lt2.seqnum)
    {
        return false;
    }
    if (lt1.data.size() != lt2.data.size())
    {
        return false;
    }
    for (size_t i = 0; i < lt1.data.size(); ++i)
    {
        if (lt1.data.at(i) != lt2.data.at(i))
        {
            return false;
        }
    }
    return true;
}

class LatencyDataType : public eprosima::fastrtps::TopicDataType
{
public:

    LatencyDataType()
    {
        setName("LatencyType");
        m_typeSize = MAX_TYPE_SIZE;
        m_isGetKeyDefined = false;
    }

    ~LatencyDataType()
    {
    }

    bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;
    bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;
    std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;
    void* createData() override;
    void deleteData(
            void* data) override;
    bool getKey(
            void* /*data*/,
            eprosima::fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool force_md5 = false) override
    {
        (void)force_md5;
        return false;
    }

};

enum TESTCOMMAND : uint32_t
{
    DEFAULT,
    READY,
    BEGIN,
    STOP,
    STOP_ERROR
};

typedef struct TestCommandType
{
    TESTCOMMAND m_command;
    TestCommandType()
    {
        m_command = DEFAULT;
    }

    TestCommandType(
            TESTCOMMAND com)
        : m_command(com)
    {
    }

}TestCommandType;

class TestCommandDataType : public eprosima::fastrtps::TopicDataType
{
public:

    TestCommandDataType()
    {
        setName("TestCommandType");
        m_typeSize = 4;
        m_isGetKeyDefined = false;
    }

    ~TestCommandDataType()
    {
    }

    bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;
    bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;
    std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;
    void* createData() override;
    void deleteData(
            void* data) override;
    bool getKey(
            void* /*data*/,
            eprosima::fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool force_md5 = false) override
    {
        (void)force_md5;
        return false;
    }

};


#endif /* LATENCYTESTTYPES_H_ */
