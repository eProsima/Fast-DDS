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
 * @file TCPReqRepHelloWorldRequester.cpp
 *
 */

#include "TCPReqRepHelloWorldRequester.hpp"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/publisher/Publisher.h>

#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TCPReqRepHelloWorldRequester::TCPReqRepHelloWorldRequester(): reply_listener_(*this), request_listener_(*this),
    current_number_(std::numeric_limits<uint16_t>::max()), number_received_(std::numeric_limits<uint16_t>::max()),
    participant_(nullptr), reply_subscriber_(nullptr), request_publisher_(nullptr),
    initialized_(false), matched_(0)
{
#if defined(PREALLOCATED_WITH_REALLOC_MEMORY_MODE_TEST)
            sattr.historyMemoryPolicy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
            puattr.historyMemoryPolicy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
#elif defined(DYNAMIC_RESERVE_MEMORY_MODE_TEST)
            sattr.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
            puattr.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
#else
            sattr.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
            puattr.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
#endif
}

TCPReqRepHelloWorldRequester::~TCPReqRepHelloWorldRequester()
{
    if(participant_ != nullptr)
        Domain::removeParticipant(participant_);
}

void TCPReqRepHelloWorldRequester::init(int participantId, int domainId)
{
    ParticipantAttributes pattr;

    int32_t kind = LOCATOR_KIND_TCPv4;

    Locator_t initial_peer_locator;
    initial_peer_locator.kind = kind;
    IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
    initial_peer_locator.port = 5100;
    pattr.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel

    Locator_t unicast_locator;
    unicast_locator.kind = kind;
    IPLocator::setIPv4(unicast_locator, "127.0.0.1");
    unicast_locator.port = 5100;
    pattr.rtps.defaultUnicastLocatorList.push_back(unicast_locator); // Subscriber's data channel

    Locator_t meta_locator;
    meta_locator.kind = kind;
    IPLocator::setIPv4(meta_locator, "127.0.0.1");
    meta_locator.port = 5100;
    pattr.rtps.builtin.metatrafficUnicastLocatorList.push_back(meta_locator); // Subscriber's meta channel

    pattr.rtps.useBuiltinTransports = false;
    std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
	descriptor->wait_for_tcp_negotiation = false;
    pattr.rtps.userTransports.push_back(descriptor);

    pattr.rtps.builtin.domainId = domainId;
    pattr.rtps.participantID = participantId;
    pattr.rtps.builtin.leaseDuration = c_TimeInfinite;
    //pattr.rtps.builtin.leaseDuration_announcementperiod = Duration_t(1, 0);
    //pattr.rtps.builtin.leaseDuration_announcementperiod = Duration_t(0, 2147483648);
    pattr.rtps.builtin.leaseDuration_announcementperiod = Duration_t(1, 0);
    participant_ = Domain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

    // Register type
    ASSERT_EQ(Domain::registerType(participant_,&type_), true);

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

void TCPReqRepHelloWorldRequester::newNumber(SampleIdentity related_sample_identity, uint16_t number)
{
    std::unique_lock<std::mutex> lock(mutex_);
    ASSERT_EQ(related_sample_identity_, related_sample_identity);
    number_received_ = number;
    ASSERT_EQ(current_number_, number_received_);
    if(current_number_ == number_received_)
        cv_.notify_one();
}

void TCPReqRepHelloWorldRequester::block(const std::chrono::seconds &seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(current_number_ != number_received_)
    {
        ASSERT_EQ(cv_.wait_for(lock, seconds), std::cv_status::no_timeout);
    }

    ASSERT_EQ(current_number_, number_received_);
}

void TCPReqRepHelloWorldRequester::waitDiscovery(bool expectMatch, int maxWait)
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ < 2)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(maxWait));

    if (expectMatch)
    {
        ASSERT_GE(matched_, 2u);
    }
    else
    {
        ASSERT_EQ(matched_, 0);
    }
}

void TCPReqRepHelloWorldRequester::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if(matched_ >= 2)
        cvDiscovery_.notify_one();
}

void TCPReqRepHelloWorldRequester::ReplyListener::onNewDataMessage(Subscriber *sub)
{
    ASSERT_NE(sub, nullptr);

    HelloWorld hello;
    SampleInfo_t info;

    if(sub->takeNextData((void*)&hello, &info))
    {
        if(info.sampleKind == ALIVE)
        {
            ASSERT_EQ(hello.message().compare("GoodBye"), 0);
            requester_.newNumber(info.related_sample_identity, hello.index());
        }
    }
}

void TCPReqRepHelloWorldRequester::send(const uint16_t number)
{
    waitDiscovery();

    WriteParams wparams;
    HelloWorld hello;
    hello.index(number);
    hello.message("HelloWorld");

    std::unique_lock<std::mutex> lock(mutex_);

    ASSERT_EQ(request_publisher_->write((void*)&hello, wparams), true);
    related_sample_identity_ = wparams.sample_identity();
    current_number_ = number;
}
