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
#include <fastrtps/utils/IPFinder.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TCPReqRepHelloWorldRequester::TCPReqRepHelloWorldRequester()
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

TCPReqRepHelloWorldRequester::~TCPReqRepHelloWorldRequester()
{
    if (participant_ != nullptr)
    {
        Domain::removeParticipant(participant_);
    }
}

void TCPReqRepHelloWorldRequester::init(
        int participantId,
        int domainId,
        uint16_t listeningPort,
        uint32_t maxInitialPeer,
        const char* certs_path,
        bool force_localhost)
{
    ParticipantAttributes pattr;

    int32_t kind = LOCATOR_KIND_TCPv4;

    eprosima::fastrtps::rtps::LocatorList_t loc;
    eprosima::fastrtps::rtps::IPFinder::getIP4Address(&loc);

    Locator_t initial_peer_locator;
    initial_peer_locator.kind = kind;
    if (!force_localhost && loc.size() > 0)
    {
        IPLocator::setIPv4(initial_peer_locator, *(loc.begin()));
    }
    else
    {
        IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
    }
    initial_peer_locator.port = listeningPort;
    pattr.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel

    pattr.rtps.useBuiltinTransports = false;
    std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
    descriptor->wait_for_tcp_negotiation = false;
    if (maxInitialPeer > 0)
    {
        descriptor->maxInitialPeersRange = maxInitialPeer;
    }

    if (certs_path != nullptr)
    {
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
        using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
        descriptor->apply_security = true;
        //descriptor->tls_config.password = "testkey";
        descriptor->tls_config.verify_file = std::string(certs_path) + "/maincacert.pem";
        descriptor->tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
        descriptor->tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
        descriptor->tls_config.add_option(TLSOptions::SINGLE_DH_USE);
        descriptor->tls_config.add_option(TLSOptions::NO_COMPRESSION);
        descriptor->tls_config.add_option(TLSOptions::NO_SSLV2);
        descriptor->tls_config.add_option(TLSOptions::NO_SSLV3);
    }

    pattr.rtps.userTransports.push_back(descriptor);

    pattr.rtps.builtin.domainId = domainId;
    pattr.rtps.participantID = participantId;
    pattr.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    //pattr.rtps.builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);
    //pattr.rtps.builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(0, 2147483648);
    pattr.rtps.builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);
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

void TCPReqRepHelloWorldRequester::newNumber(
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

void TCPReqRepHelloWorldRequester::block()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() -> bool {
        return current_number_ == number_received_;
    });

    ASSERT_EQ(related_sample_identity_, received_sample_identity_);
}

void TCPReqRepHelloWorldRequester::wait_discovery(
        std::chrono::seconds timeout)
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Requester waiting for discovery..." << std::endl;

    if (timeout == std::chrono::seconds::zero())
    {
        cvDiscovery_.wait(lock, [&](){
            return matched_ > 1;
        });
    }
    else
    {
        cvDiscovery_.wait_for(lock, timeout, [&](){
            return matched_ > 1;
        });
    }

    std::cout << "Requester discovery phase finished" << std::endl;
}

void TCPReqRepHelloWorldRequester::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (matched_ > 1)
    {
        cvDiscovery_.notify_one();
    }
}

bool TCPReqRepHelloWorldRequester::is_matched()
{
    return matched_ > 1;
}

void TCPReqRepHelloWorldRequester::ReplyListener::onNewDataMessage(
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

void TCPReqRepHelloWorldRequester::send(
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
}
