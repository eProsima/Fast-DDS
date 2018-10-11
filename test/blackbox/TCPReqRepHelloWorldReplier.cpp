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
 * @file TCPReqRepHelloWorldReplier.cpp
 *
 */

#include "TCPReqRepHelloWorldReplier.hpp"

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

TCPReqRepHelloWorldReplier::TCPReqRepHelloWorldReplier(): request_listener_(*this), reply_listener_(*this),
    participant_(nullptr), request_subscriber_(nullptr), reply_publisher_(nullptr),
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

TCPReqRepHelloWorldReplier::~TCPReqRepHelloWorldReplier()
{
    if(participant_ != nullptr)
        Domain::removeParticipant(participant_);
}

void TCPReqRepHelloWorldReplier::init(int participantId, int domainId, uint16_t listeningPort)
{
    ParticipantAttributes pattr;
    pattr.rtps.builtin.domainId = domainId;
    pattr.rtps.participantID = participantId;
    pattr.rtps.builtin.leaseDuration = c_TimeInfinite;
    //pattr.rtps.builtin.leaseDuration_announcementperiod = Duration_t(1, 0);
    //pattr.rtps.builtin.leaseDuration_announcementperiod = Duration_t(0, 2147483648);
    pattr.rtps.builtin.leaseDuration_announcementperiod = Duration_t(1, 0);

    // TCP CONNECTION PEER.
    //uint32_t kind = LOCATOR_KIND_TCPv4;

    pattr.rtps.useBuiltinTransports = false;

    std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
    descriptor->wait_for_tcp_negotiation = false;
    descriptor->sendBufferSize = 0;
    descriptor->receiveBufferSize = 0;
    // descriptor->set_WAN_address("127.0.0.1");
    descriptor->add_listener_port(listeningPort);
    pattr.rtps.userTransports.push_back(descriptor);

    participant_ = Domain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

    // Register type
    ASSERT_EQ(Domain::registerType(participant_,&type_), true);

    //Create subscriber
    sattr.topic.topicKind = NO_KEY;
    sattr.topic.topicDataType = "HelloWorldType";
    configSubscriber("Request");
    request_subscriber_ = Domain::createSubscriber(participant_, sattr, &request_listener_);
    ASSERT_NE(request_subscriber_, nullptr);

    //Create publisher
    puattr.topic.topicKind = NO_KEY;
    puattr.topic.topicDataType = "HelloWorldType";
    puattr.topic.topicName = "HelloWorldTopicReply";
    configPublisher("Reply");
    reply_publisher_ = Domain::createPublisher(participant_, puattr, &reply_listener_);
    ASSERT_NE(reply_publisher_, nullptr);

    initialized_ = true;
}

void TCPReqRepHelloWorldReplier::newNumber(SampleIdentity sample_identity, uint16_t number)
{
    WriteParams wparams;
    HelloWorld hello;
    hello.index(number);
    hello.message("GoodBye");
    wparams.related_sample_identity(sample_identity);
    ASSERT_EQ(reply_publisher_->write((void*)&hello, wparams), true);
}

void TCPReqRepHelloWorldReplier::waitDiscovery()
{
    std::cout << "Replier waiting for discovery..." << std::endl;
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ == 0)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

    std::cout << "Replier discovery phase finished" << std::endl;
}

void TCPReqRepHelloWorldReplier::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if(matched_ > 1)
        cvDiscovery_.notify_one();
}

void TCPReqRepHelloWorldReplier::ReplyListener::onNewDataMessage(Subscriber *sub)
{
    ASSERT_NE(sub, nullptr);

    HelloWorld hello;
    SampleInfo_t info;

    if(sub->takeNextData((void*)&hello, &info))
    {
        if(info.sampleKind == ALIVE)
        {
            ASSERT_EQ(hello.message().compare("HelloWorld"), 0);
            replier_.newNumber(info.sample_identity, hello.index());
        }
    }
}
