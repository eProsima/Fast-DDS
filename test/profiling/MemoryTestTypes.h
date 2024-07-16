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
 * @file MemoryTestTypes.h
 *
 */

#ifndef MEMORYTESTTYPES_H_
#define MEMORYTESTTYPES_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include <fastdds/dds/topic/TopicDataType.hpp>

class MemoryType
{
public:

    uint32_t seqnum;
    std::vector<uint8_t> data;

    MemoryType()
        : seqnum(0)
    {
    }

    MemoryType(
            uint32_t number)
        : seqnum(0)
        , data(number, 0)
    {
    }

    ~MemoryType()
    {
    }

};


inline bool operator ==(
        const MemoryType& lt1,
        const MemoryType& lt2)
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

class MemoryDataType : public eprosima::fastdds::dds::TopicDataType
{
public:

    MemoryDataType()
    {
        set_name("MemoryType");
        max_serialized_type_size = 17000;
        is_compute_key_provided = false;
    }

    ~MemoryDataType()
    {
    }

    bool serialize(
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;
    bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override;
    uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;
    void* create_data() override;
    void delete_data(
            void* data) override;
    bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& /*payload*/,
            eprosima::fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool force_md5 = false) override
    {
        (void)force_md5;
        return false;
    }

    bool compute_key(
            const void* const /*data*/,
            eprosima::fastdds::rtps::InstanceHandle_t& /*ihandle*/,
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

class TestCommandDataType : public eprosima::fastdds::dds::TopicDataType
{
public:

    TestCommandDataType()
    {
        set_name("TestCommandType");
        max_serialized_type_size = 4;
        is_compute_key_provided = false;
    }

    ~TestCommandDataType()
    {
    }

    bool serialize(
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;
    bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override;
    uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;
    void* create_data() override;
    void delete_data(
            void* data) override;

    bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& /*payload*/,
            eprosima::fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool force_md5 = false) override
    {
        (void)force_md5;
        return false;
    }

    bool compute_key(
            const void* const /*data*/,
            eprosima::fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool force_md5 = false) override
    {
        (void)force_md5;
        return false;
    }

};


#endif /* MEMORYTESTTYPES_H_ */
