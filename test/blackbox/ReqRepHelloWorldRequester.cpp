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

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/publisher/Publisher.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

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
    sattr.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
    puattr.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
}

ReqRepHelloWorldRequester::~ReqRepHelloWorldRequester()
{
    if (participant_ != nullptr)
    {
        Domain::removeParticipant(participant_);
    }
}

void ReqRepHelloWorldRequester::init()
{
    ParticipantAttributes pattr;
    pattr.domainId = (uint32_t)GET_PID() % 230;
    participant_ = Domain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

    // Register type
    ASSERT_EQ(Domain::registerType(participant_, &type_), true);

    //Create subscriber
    sattr.topic.topicKind = NO_KEY;
    sattr.topic.topicDataType = "HelloWorldType";
    configSubscriber("Reply");
    reply_subscriber_ = Domain::createSubscriber(participant_, sattr, &reply_listener_);
    ASSERT_NE(reply_subscriber_, nullptr);

    //Create publisher
    puattr.topic.topicKind = NO_KEY;
    puattr.topic.topicDataType = "HelloWorldType";
    configPublisher("Request");
    request_publisher_ = Domain::createPublisher(participant_, puattr, &request_listener_);
    ASSERT_NE(request_publisher_, nullptr);

    initialized_ = true;
}

void ReqRepHelloWorldRequester::init_with_latency(
        const eprosima::fastrtps::Duration_t& latency_budget_duration_pub,
        const eprosima::fastrtps::Duration_t& latency_budget_duration_sub)
{
    ParticipantAttributes pattr;
    participant_ = Domain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

    // Register type
    ASSERT_EQ(Domain::registerType(participant_, &type_), true);

    //Create subscriber
    sattr.topic.topicKind = NO_KEY;
    sattr.topic.topicDataType = "HelloWorldType";
    sattr.qos.m_latencyBudget.duration = latency_budget_duration_sub;
    reply_subscriber_ = Domain::createSubscriber(participant_, sattr, &reply_listener_);
    ASSERT_NE(reply_subscriber_, nullptr);

    //Create publisher
    puattr.topic.topicKind = NO_KEY;
    puattr.topic.topicDataType = "HelloWorldType";
    puattr.qos.m_latencyBudget.duration = latency_budget_duration_pub;
    request_publisher_ = Domain::createPublisher(participant_, puattr, &request_listener_);
    ASSERT_NE(request_publisher_, nullptr);

    initialized_ = true;
}

void ReqRepHelloWorldRequester::newNumber(
        SampleIdentity related_sample_identity,
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

void ReqRepHelloWorldRequester::ReplyListener::onNewDataMessage(
        Subscriber* sub)
{
    ASSERT_NE(sub, nullptr);

    HelloWorld hello;
    SampleInfo_t info;

    if (sub->takeNextData((void*)&hello, &info))
    {
        if (info.sampleKind == ALIVE)
        {
            ASSERT_EQ(hello.message().compare("GoodBye"), 0);
            requester_.newNumber(info.related_sample_identity, hello.index());
        }
    }
}

void ReqRepHelloWorldRequester::send(
        const uint16_t number)
{
    WriteParams wparams;
    HelloWorld hello;
    hello.index(number);
    hello.message("HelloWorld");

    {
        std::unique_lock<std::mutex> lock(mutex_);
        current_number_ = number;
    }

    ASSERT_EQ(request_publisher_->write((void*)&hello, wparams), true);
    related_sample_identity_ = wparams.sample_identity();
    ASSERT_NE(related_sample_identity_.sequence_number(), SequenceNumber_t());
}
