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

#include "../../common/BlackboxTests.hpp"
#include "TCPReqRepHelloWorldRequester.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>

#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/IPFinder.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;

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
    // By default, memory mode is PREALLOCATED_WITH_REALLOC_MEMORY_MODE
    datareader_qos_.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    datawriter_qos_.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
}

TCPReqRepHelloWorldRequester::~TCPReqRepHelloWorldRequester()
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

void TCPReqRepHelloWorldRequester::init(
        int participantId,
        int domainId,
        uint16_t listeningPort,
        uint32_t maxInitialPeer,
        const char* certs_folder,
        bool force_localhost)
{
    DomainParticipantQos participant_qos;

    int32_t kind;
    LocatorList_t loc;

    if (use_ipv6)
    {
        kind = LOCATOR_KIND_TCPv6;
        IPFinder::getIP6Address(&loc);

    }
    else
    {
        kind = LOCATOR_KIND_TCPv4;
        IPFinder::getIP4Address(&loc);

    }


    Locator_t initial_peer_locator;
    initial_peer_locator.kind = kind;
    if (!force_localhost && loc.size() > 0)
    {
        if (use_ipv6)
        {
            IPLocator::setIPv6(initial_peer_locator, *(loc.begin()));
        }
        else
        {
            IPLocator::setIPv4(initial_peer_locator, *(loc.begin()));
        }
    }
    else
    {
        if (use_ipv6)
        {
            IPLocator::setIPv6(initial_peer_locator, "::1");
        }
        else
        {
            IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
        }
    }

    initial_peer_locator.port = listeningPort;
    participant_qos.wire_protocol().builtin.initialPeersList.push_back(initial_peer_locator);
    participant_qos.transport().use_builtin_transports = false;

    std::shared_ptr<TCPTransportDescriptor> descriptor;
    if (use_ipv6)
    {
        descriptor = std::make_shared<TCPv6TransportDescriptor>();
    }
    else
    {
        descriptor = std::make_shared<TCPv4TransportDescriptor>();
    }

    if (maxInitialPeer > 0)
    {
        descriptor->maxInitialPeersRange = maxInitialPeer;
    }

    if (certs_folder != nullptr)
    {
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
        using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
        descriptor->apply_security = true;
        //descriptor->tls_config.password = "testkey";
        descriptor->tls_config.verify_file = std::string(certs_folder) + "/maincacert.pem";
        descriptor->tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
        descriptor->tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
        descriptor->tls_config.add_option(TLSOptions::SINGLE_DH_USE);
        descriptor->tls_config.add_option(TLSOptions::NO_COMPRESSION);
        descriptor->tls_config.add_option(TLSOptions::NO_SSLV2);
    }

    participant_qos.transport().user_transports.push_back(descriptor);

    participant_qos.wire_protocol().participant_id = participantId;
    participant_qos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    participant_qos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);

    participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        domainId, participant_qos);
    ASSERT_NE(participant_, nullptr);
    ASSERT_TRUE(participant_->is_enabled());

    // Register type
    type_.reset(new HelloWorldPubSubType());
    ASSERT_EQ(participant_->register_type(type_), ReturnCode_t::RETCODE_OK);

    //Create subscriber
    reply_subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(reply_subscriber_, nullptr);
    ASSERT_TRUE(reply_subscriber_->is_enabled());

    //Create publisher
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
    datareader_qos_.reliability().kind = RELIABLE_RELIABILITY_QOS;
    //Increase default max_blocking_time to 1s in case the CPU is overhead
    datareader_qos_.reliability().max_blocking_time = Duration_t(1, 0);
    reply_datareader_ = reply_subscriber_->create_datareader(reply_topic_, datareader_qos_, &reply_listener_);
    ASSERT_NE(reply_datareader_, nullptr);
    ASSERT_TRUE(reply_datareader_->is_enabled());

    //Create DataWriter
    datawriter_qos_.reliability().kind = RELIABLE_RELIABILITY_QOS;
    datawriter_qos_.reliability().max_blocking_time = Duration_t(1, 0);
    request_datawriter_ = request_publisher_->create_datawriter(request_topic_, datawriter_qos_,
                    &request_listener_);
    ASSERT_NE(request_datawriter_, nullptr);
    ASSERT_TRUE(request_datawriter_->is_enabled());

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
    cv_.wait(lock, [this]() -> bool
            {
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
        cvDiscovery_.wait(lock, [&]()
                {
                    return is_matched();
                });
    }
    else
    {
        cvDiscovery_.wait_for(lock, timeout, [&]()
                {
                    return is_matched();
                });
    }

    std::cout << "Requester discovery phase finished" << std::endl;
}

void TCPReqRepHelloWorldRequester::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (is_matched())
    {
        cvDiscovery_.notify_one();
    }
}

void TCPReqRepHelloWorldRequester::unmatched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    --matched_;
}

bool TCPReqRepHelloWorldRequester::is_matched()
{
    return matched_ > 1;
}

void TCPReqRepHelloWorldRequester::ReplyListener::on_data_available(
        eprosima::fastdds::dds::DataReader* datareader)
{
    ASSERT_NE(datareader, nullptr);

    HelloWorld hello;
    eprosima::fastdds::dds::SampleInfo info;

    if (ReturnCode_t::RETCODE_OK == datareader->take_next_sample((void*)&hello, &info))
    {
        if (info.valid_data)
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

    ASSERT_EQ(request_datawriter_->write((void*)&hello, wparams), true);
    related_sample_identity_ = wparams.sample_identity();
    ASSERT_NE(related_sample_identity_.sequence_number(), SequenceNumber_t());
}
