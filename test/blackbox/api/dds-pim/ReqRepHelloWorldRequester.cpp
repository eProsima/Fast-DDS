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
 * @file ReqRepHelloWorldRequester.cpp
 *
 */

#include "ReqRepHelloWorldRequester.hpp"

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

ReqRepHelloWorldRequester::ReqRepHelloWorldRequester()
    : reply_listener_(*this)
    , request_listener_(*this)
    , current_number_(std::numeric_limits<uint16_t>::max())
    , number_received_(std::numeric_limits<uint16_t>::max())
    , participant_(nullptr)
    , reply_subscriber_(nullptr)
    , request_publisher_(nullptr)
    , initialized_(false)
    , matched_(0)
{
    // By default, memory mode is preallocated (the most restritive)
    datareader_qos_.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
    datawriter_qos_.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
}

ReqRepHelloWorldRequester::~ReqRepHelloWorldRequester()
{
    if (participant_ != nullptr)
    {
        if (reply_subscriber_)
        {
            if (reply_datareader_)
            {
                reply_subscriber_->delete_datareader(reply_datareader_);
            }
            participant_->delete_subscriber(reply_subscriber_);
        }
        if (request_publisher_)
        {
            if (request_datawriter_)
            {
                request_publisher_->delete_datawriter(request_datawriter_);
            }
            participant_->delete_publisher(request_publisher_);
        }
        if (request_topic_)
        {
            participant_->delete_topic(request_topic_);
        }
        if (reply_topic_)
        {
            participant_->delete_topic(reply_topic_);
        }
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void ReqRepHelloWorldRequester::init()
{
    participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        (uint32_t)GET_PID() % 230, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant_, nullptr);
    ASSERT_TRUE(participant_->is_enabled());

    // Register type
    type_.reset(new HelloWorldType());
    ASSERT_EQ(participant_->register_type(type_), ReturnCode_t::RETCODE_OK);

    reply_subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(reply_subscriber_, nullptr);
    ASSERT_TRUE(reply_subscriber_->is_enabled());

    request_publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(request_publisher_, nullptr);
    ASSERT_TRUE(request_publisher_->is_enabled());

    configDatareader("Reply");
    reply_topic_ = participant_->create_topic(datareader_topicname_,
                    type_->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    ASSERT_NE(reply_topic_, nullptr);
    ASSERT_TRUE(reply_topic_->is_enabled());

    configDatawriter("Request");
    request_topic_ = participant_->create_topic(datawriter_topicname_,
                    type_->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    ASSERT_NE(request_topic_, nullptr);
    ASSERT_TRUE(request_topic_->is_enabled());

    //Create DataReader
    reply_datareader_ = reply_subscriber_->create_datareader(reply_topic_, datareader_qos_, &reply_listener_);
    ASSERT_NE(reply_datareader_, nullptr);
    ASSERT_TRUE(reply_datareader_->is_enabled());

    //Create DataWriter
    request_datawriter_ = request_publisher_->create_datawriter(request_topic_, datawriter_qos_,
                    &request_listener_);
    ASSERT_NE(request_datawriter_, nullptr);
    ASSERT_TRUE(request_datawriter_->is_enabled());

    initialized_ = true;
}

void ReqRepHelloWorldRequester::init_with_latency(
        const eprosima::fastrtps::Duration_t& latency_budget_duration_pub,
        const eprosima::fastrtps::Duration_t& latency_budget_duration_sub)
{
    datareader_qos_.latency_budget().duration = latency_budget_duration_sub;
    datawriter_qos_.latency_budget().duration = latency_budget_duration_pub;
    init();
}

void ReqRepHelloWorldRequester::newNumber(
        eprosima::fastrtps::rtps::SampleIdentity related_sample_identity,
        uint16_t number)
{
    std::unique_lock<std::mutex> lock(mutex_);
    received_sample_identity_ = related_sample_identity;
    number_received_ = number;
    ASSERT_EQ(current_number_, number_received_);
    if (current_number_ == number_received_)
    {
        cv_.notify_one();
    }
}

void ReqRepHelloWorldRequester::block(
        const std::chrono::seconds& seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);

    bool timeout = cv_.wait_for(lock, seconds, [&]() -> bool
    {
        return current_number_ == number_received_;
    });

    ASSERT_TRUE(timeout);
    ASSERT_EQ(current_number_, number_received_);
    ASSERT_EQ(related_sample_identity_, received_sample_identity_);
}

void ReqRepHelloWorldRequester::wait_discovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Requester is waiting discovery..." << std::endl;

    cvDiscovery_.wait(lock, [&](){
        return matched_ > 1;
    });

    std::cout << "Requester discovery finished..." << std::endl;
}

void ReqRepHelloWorldRequester::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (matched_ > 1)
    {
        cvDiscovery_.notify_one();
    }
}

void ReqRepHelloWorldRequester::ReplyListener::on_data_available(
        eprosima::fastdds::dds::DataReader* datareader)
{
    ASSERT_NE(datareader, nullptr);

    HelloWorld hello;
    eprosima::fastdds::dds::SampleInfo info;

    if (ReturnCode_t::RETCODE_OK == datareader->take_next_sample((void*)&hello, &info))
    {
        if (info.instance_state == eprosima::fastdds::dds::InstanceStateKind::ALIVE)
        {
            ASSERT_EQ(hello.message().compare("GoodBye"), 0);
            requester_.newNumber(info.related_sample_identity, hello.index());
        }
    }
}

void ReqRepHelloWorldRequester::send(
        const uint16_t number)
{
    eprosima::fastrtps::rtps::WriteParams wparams;
    HelloWorld hello;
    hello.index(number);
    hello.message("HelloWorld");

    {
        std::unique_lock<std::mutex> lock(mutex_);
        current_number_ = number;
    }

    ASSERT_EQ(request_datawriter_->write((void*)&hello, wparams), true);
    related_sample_identity_ = wparams.sample_identity();
    ASSERT_NE(related_sample_identity_.sequence_number(), eprosima::fastrtps::rtps::SequenceNumber_t());
}
