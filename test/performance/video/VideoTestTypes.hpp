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
 * @file VideoTestTypes.h
 *
 */

#ifndef VIDEOTESTTYPES_H_
#define VIDEOTESTTYPES_H_

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>

#include <fastdds/utils/IPFinder.hpp>



class VideoType
{
public:

    uint32_t seqnum;
    uint64_t timestamp;
    uint64_t duration;
    std::vector<uint8_t> data;

    VideoType()
        : seqnum(0)
        , timestamp(0)
        , duration(0)
    {
    }

    VideoType(
            uint32_t number)
        : seqnum(0)
        , timestamp(0)
        , duration(0)
        , data(number, 0)
    {
    }

    ~VideoType()
    {
    }

};


inline bool operator ==(
        const VideoType& lt1,
        const VideoType& lt2)
{
    if (lt1.seqnum != lt2.seqnum)
    {
        return false;
    }
    if (lt1.timestamp != lt2.timestamp)
    {
        return false;
    }
    if (lt1.duration != lt2.duration)
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

class VideoDataType : public eprosima::fastdds::dds::TopicDataType
{
public:

    VideoDataType()
    {
        setName("VideoType");
        m_typeSize = 17000;
        m_isGetKeyDefined = false;
    }

    ~VideoDataType()
    {
    }

    bool serialize(
            void* data,
            eprosima::fastdds::rtps::SerializedPayload_t* payload) override;
    bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t* payload,
            void* data) override;
    std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;
    void* createData() override;
    void deleteData(
            void* data) override;
    bool getKey(
            void* /*data*/,
            eprosima::fastdds::rtps::InstanceHandle_t* /*ihandle*/,
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
        setName("TestCommandType");
        m_typeSize = 4;
        m_isGetKeyDefined = false;
    }

    ~TestCommandDataType()
    {
    }

    bool serialize(
            void* data,
            eprosima::fastdds::rtps::SerializedPayload_t* payload) override;
    bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t* payload,
            void* data) override;
    std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;
    void* createData() override;
    void deleteData(
            void* data) override;
    bool getKey(
            void* /*data*/,
            eprosima::fastdds::rtps::InstanceHandle_t* /*ihandle*/,
            bool force_md5 = false) override
    {
        (void)force_md5;
        return false;
    }

};


#endif /* VIDEOTESTTYPES_H_ */
