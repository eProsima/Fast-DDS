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
 * @file ReqRepHelloWorldReplier.cpp
 *
 */

#include "ReqRepHelloWorldReplier.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

#include <gtest/gtest.h>

ReqRepHelloWorldReplier::ReqRepHelloWorldReplier()
    : request_listener_(*this)
    , reply_listener_(*this)
    , participant_(nullptr)
    , request_topic_(nullptr)
    , request_subscriber_(nullptr)
    , request_datareader_(nullptr)
    , reply_topic_(nullptr)
    , reply_publisher_(nullptr)
    , reply_datawriter_(nullptr)
    , initialized_(false)
    , matched_(0)
{
    // By default, memory mode is preallocated (the most restritive)
    datareader_qos_.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
    datawriter_qos_.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
}

ReqRepHelloWorldReplier::~ReqRepHelloWorldReplier()
{
    if (request_datareader_)
    {
        request_subscriber_->delete_datareader(request_datareader_);
    }
    if (reply_datawriter_)
    {
        reply_publisher_->delete_datawriter(reply_datawriter_);
    }
    if (request_subscriber_)
    {
        participant_->delete_subscriber(request_subscriber_);
    }
    if (reply_publisher_)
    {
        participant_->delete_publisher(reply_publisher_);
    }
    if (request_topic_)
    {
        participant_->delete_topic(request_topic_);
    }
    if (reply_topic_)
    {
        participant_->delete_topic(reply_topic_);
    }
    if (participant_ != nullptr)
    {
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void ReqRepHelloWorldReplier::init()
{
    participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant_, nullptr);

    // Register type
    type_.reset(new HelloWorldType());
    ASSERT_EQ(participant_->register_type(type_), true);

    request_topic_ = participant_->create_topic(datareader_topicname_,
                    type_->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    ASSERT_NE(request_topic_, nullptr);

    reply_topic_ = participant_->create_topic(datawriter_topicname_,
                    type_->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    ASSERT_NE(reply_topic_, nullptr);

    request_subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(request_subscriber_, nullptr);

    reply_publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(reply_publisher_, nullptr);

    //Create datareader
    configDatareader("Request");
    request_datareader_ = request_subscriber_->create_datareader(request_topic_, datareader_qos_,
                    &request_listener_);
    ASSERT_NE(request_datareader_, nullptr);

    //Create publisher
    configDatawriter("Reply");
    reply_datawriter_ = reply_publisher_->create_datawriter(reply_topic_, datawriter_qos_, &reply_listener_);
    ASSERT_NE(reply_datawriter_, nullptr);

    initialized_ = true;
}

void ReqRepHelloWorldReplier::newNumber(
        eprosima::fastrtps::rtps::SampleIdentity sample_identity,
        uint16_t number)
{
    eprosima::fastrtps::rtps::WriteParams wparams;
    HelloWorld hello;
    hello.index(number);
    hello.message("GoodBye");
    wparams.related_sample_identity(sample_identity);
    ASSERT_EQ(reply_datawriter_->write((void*)&hello, wparams), true);
}

void ReqRepHelloWorldReplier::wait_discovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Replier is waiting discovery..." << std::endl;

    cvDiscovery_.wait(lock, [&](){
        return matched_ > 1;
    });

    std::cout << "Replier discovery finished..." << std::endl;
}

void ReqRepHelloWorldReplier::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (matched_ > 1)
    {
        cvDiscovery_.notify_one();
    }
}

void ReqRepHelloWorldReplier::ReplyListener::on_data_available(
        eprosima::fastdds::dds::DataReader* datareader)
{
    ASSERT_NE(datareader, nullptr);

    HelloWorld hello;
    eprosima::fastdds::dds::SampleInfo info;

    if (ReturnCode_t::RETCODE_OK == datareader->take_next_sample((void*)&hello, &info))
    {
        if (info.instance_state == eprosima::fastdds::dds::InstanceStateKind::ALIVE)
        {
            ASSERT_EQ(hello.message().compare("HelloWorld"), 0);
            replier_.newNumber(info.sample_identity, hello.index());
        }
    }
}
