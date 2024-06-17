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

#include "../../common/BlackboxTests.hpp"
#include "TCPReqRepHelloWorldReplier.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastdds/utils/IPLocator.h>

#include <gtest/gtest.h>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;

TCPReqRepHelloWorldReplier::TCPReqRepHelloWorldReplier()
    : request_listener_(*this)
    , reply_listener_(*this)
    , participant_(nullptr)
    , request_subscriber_(nullptr)
    , reply_publisher_(nullptr)
    , initialized_(false)
    , matched_(0)
{
    // By default, memory mode is PREALLOCATED_WITH_REALLOC_MEMORY_MODE
    datareader_qos_.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    datawriter_qos_.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
}

TCPReqRepHelloWorldReplier::~TCPReqRepHelloWorldReplier()
{
    if (participant_ != nullptr)
    {
        if (request_subscriber_)
        {
            if (request_datareader_)
            {
                request_subscriber_->delete_datareader(request_datareader_);
            }
            participant_->delete_subscriber(request_subscriber_);
        }
        if (reply_publisher_)
        {
            if (reply_datawriter_)
            {
                reply_publisher_->delete_datawriter(reply_datawriter_);
            }
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
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void TCPReqRepHelloWorldReplier::init(
        int participantId,
        int domainId,
        uint16_t listeningPort,
        uint32_t maxInitialPeer,
        const char* certs_folder)
{
    DomainParticipantQos participant_qos;
    participant_qos.wire_protocol().participant_id = participantId;
    participant_qos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);
    participant_qos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastdds::c_TimeInfinite;

    participant_qos.transport().use_builtin_transports = false;

    std::shared_ptr<TCPTransportDescriptor> descriptor;
    if (use_ipv6)
    {
        descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
    }
    else
    {
        descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    }

    descriptor->sendBufferSize = 0;
    descriptor->receiveBufferSize = 0;
    if (maxInitialPeer > 0)
    {
        descriptor->maxInitialPeersRange = maxInitialPeer;
    }
    descriptor->add_listener_port(listeningPort);

    if (certs_folder != nullptr)
    {
        using TLSOptions = eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions;
        using TLSVerifyMode = eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
        descriptor->apply_security = true;
        descriptor->tls_config.password = "testkey";
        descriptor->tls_config.cert_chain_file = std::string(certs_folder) + "/mainsubcert.pem";
        descriptor->tls_config.private_key_file = std::string(certs_folder) + "/mainsubkey.pem";
        descriptor->tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
        descriptor->tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
        descriptor->tls_config.add_option(TLSOptions::SINGLE_DH_USE);
        descriptor->tls_config.add_option(TLSOptions::NO_COMPRESSION);
        descriptor->tls_config.add_option(TLSOptions::NO_SSLV2);
    }

    participant_qos.transport().user_transports.push_back(descriptor);

    participant_ = DomainParticipantFactory::get_instance()->create_participant(
        domainId, participant_qos);
    ASSERT_NE(participant_, nullptr);
    ASSERT_TRUE(participant_->is_enabled());

    // Register type
    type_.reset(new HelloWorldPubSubType());
    ASSERT_EQ(participant_->register_type(type_), RETCODE_OK);

    configDatareader("Request");
    request_topic_ = participant_->create_topic(datareader_topicname_,
                    type_->getName(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(request_topic_, nullptr);
    ASSERT_TRUE(request_topic_->is_enabled());

    configDatawriter("Reply");
    reply_topic_ = participant_->create_topic(datawriter_topicname_,
                    type_->getName(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(reply_topic_, nullptr);
    ASSERT_TRUE(reply_topic_->is_enabled());

    //Create subscriber
    request_subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(request_subscriber_, nullptr);
    ASSERT_TRUE(request_subscriber_->is_enabled());

    //Create publisher
    reply_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(reply_publisher_, nullptr);
    ASSERT_TRUE(reply_publisher_->is_enabled());

    //Create datareader
    datareader_qos_.reliability().kind = RELIABLE_RELIABILITY_QOS;
    //Increase default max_blocking_time to 1s in case the CPU is overhead
    datareader_qos_.reliability().max_blocking_time = Duration_t(1, 0);
    request_datareader_ = request_subscriber_->create_datareader(request_topic_, datareader_qos_,
                    &request_listener_);
    ASSERT_NE(request_datareader_, nullptr);
    ASSERT_TRUE(request_datareader_->is_enabled());

    //Create datawriter
    datawriter_qos_.reliability().kind = RELIABLE_RELIABILITY_QOS;
    datawriter_qos_.reliability().max_blocking_time = Duration_t(1, 0);
    reply_datawriter_ = reply_publisher_->create_datawriter(reply_topic_, datawriter_qos_, &reply_listener_);
    ASSERT_NE(reply_datawriter_, nullptr);
    ASSERT_TRUE(reply_datawriter_->is_enabled());

    initialized_ = true;
}

void TCPReqRepHelloWorldReplier::newNumber(
        SampleIdentity sample_identity,
        uint16_t number)
{
    WriteParams wparams;
    HelloWorld hello;
    hello.index(number);
    hello.message("GoodBye");
    wparams.related_sample_identity(sample_identity);
    ASSERT_EQ(reply_datawriter_->write((void*)&hello, wparams), true);
}

void TCPReqRepHelloWorldReplier::wait_discovery(
        std::chrono::seconds timeout)
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Replier waiting for discovery..." << std::endl;

    if (timeout == std::chrono::seconds::zero())
    {
        cvDiscovery_.wait(lock, [&]()
                {
                    return matched_ > 1;
                });
    }
    else
    {
        cvDiscovery_.wait_for(lock, timeout, [&]()
                {
                    return matched_ > 1;
                });
    }

    std::cout << "Replier discovery phase finished" << std::endl;
}

void TCPReqRepHelloWorldReplier::wait_unmatched(
        std::chrono::seconds timeout)
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    std::cout << "Replier waiting until being unmatched..." << std::endl;

    if (timeout == std::chrono::seconds::zero())
    {
        cvDiscovery_.wait(lock, [&]()
                {
                    return !is_matched();
                });
    }
    else
    {
        cvDiscovery_.wait_for(lock, timeout, [&]()
                {
                    return !is_matched();
                });
    }

    std::cout << "Replier unmatched" << std::endl;
}

void TCPReqRepHelloWorldReplier::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if (matched_ > 1)
    {
        cvDiscovery_.notify_one();
    }
}

void TCPReqRepHelloWorldReplier::unmatched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    --matched_;
    if (!is_matched())
    {
        cvDiscovery_.notify_one();
    }
}

bool TCPReqRepHelloWorldReplier::is_matched()
{
    return matched_ > 1;
}

void TCPReqRepHelloWorldReplier::ReplyListener::on_data_available(
        DataReader* datareader)
{
    ASSERT_NE(datareader, nullptr);

    HelloWorld hello;
    SampleInfo info;

    if (RETCODE_OK == datareader->take_next_sample((void*)&hello, &info))
    {
        if (info.valid_data)
        {
            ASSERT_EQ(hello.message().compare("HelloWorld"), 0);
            replier_.newNumber(info.sample_identity, hello.index());
        }
    }
}
