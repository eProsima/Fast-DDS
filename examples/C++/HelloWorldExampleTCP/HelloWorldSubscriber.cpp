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
 * @file HelloWorldSubscriber.cpp
 *
 */

#include "HelloWorldSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/IPLocator.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
{
}

bool HelloWorldSubscriber::init(
        const std::string& wan_ip,
        unsigned short port,
        bool use_tls,
        const std::vector<std::string>& whitelist)
{
    ParticipantAttributes pparam;
    int32_t kind = LOCATOR_KIND_TCPv4;

    Locator_t initial_peer_locator;
    initial_peer_locator.kind = kind;

    std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();

    for (std::string ip : whitelist)
    {
        descriptor->interfaceWhiteList.push_back(ip);
        std::cout << "Whitelisted " << ip << std::endl;
    }

    if (!wan_ip.empty())
    {
        IPLocator::setIPv4(initial_peer_locator, wan_ip);
        std::cout << wan_ip << ":" << port << std::endl;
    }
    else
    {
        IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
    }
    initial_peer_locator.port = port;
    pparam.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel

    pparam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    pparam.rtps.builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(5, 0);
    pparam.rtps.setName("Participant_sub");

    pparam.rtps.useBuiltinTransports = false;

    if (use_tls)
    {
        using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
        descriptor->apply_security = true;
        descriptor->tls_config.password = "test";
        descriptor->tls_config.verify_file = "ca.pem";
        descriptor->tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
        descriptor->tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
    }

    descriptor->wait_for_tcp_negotiation = false;
    pparam.rtps.userTransports.push_back(descriptor);

    participant_ = Domain::createParticipant(pparam);
    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    Domain::registerType(participant_, &type_);

    //CREATE THE SUBSCRIBER
    SubscriberAttributes rparam;
    rparam.topic.topicKind = NO_KEY;
    rparam.topic.topicDataType = "HelloWorld";
    rparam.topic.topicName = "HelloWorldTopicTCP";
    rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    rparam.topic.historyQos.depth = 30;
    rparam.topic.resourceLimitsQos.max_samples = 50;
    rparam.topic.resourceLimitsQos.allocated_samples = 20;
    rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    subscriber_ = Domain::createSubscriber(participant_, rparam, (SubscriberListener*)&listener);
    if (subscriber_ == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    Domain::removeParticipant(participant_);
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(
        Subscriber*,
        MatchingInfo& matching_info)
{
    if (matching_info.status == MATCHED_MATCHING)
    {
        n_matched++;
        //logError(HW, "Matched");
        std::cout << "[RTCP] Subscriber matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "[RTCP] Subscriber unmatched" << std::endl;
    }
}

void HelloWorldSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData((void*)&hello, &info))
    {
        if (info.sampleKind == ALIVE)
        {
            this->n_samples++;
            // Print your structure data here.
            //logError(HW, "RECEIVED " <<  hello.index());
            std::cout << "[RTCP] Message " << hello.message() << " " << hello.index() << " RECEIVED" << std::endl;
        }
    }
}

void HelloWorldSubscriber::run()
{
    std::cout << "[RTCP] Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void HelloWorldSubscriber::run(
        uint32_t number)
{
    std::cout << "[RTCP] Subscriber running until " << number << "samples have been received" << std::endl;
    while (number < this->listener.n_samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
