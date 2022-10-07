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

#include <thread>
#include <string>

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include "common.hpp"

using namespace eprosima::fastdds::dds;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType())
    , host_name_(asio::ip::host_name())
{
}

bool HelloWorldSubscriber::init(
        bool use_env,
        eprosima::examples::helloworld::AutomaticDiscovery discovery_mode,
        const eprosima::fastdds::rtps::LocatorList& initial_peers)
{
    discovery_mode_ = discovery_mode;
    std::cout << "Subscriber discovery mode: " << discovery_mode_ << std::endl;
    if (!initial_peers.empty())
    {
        std::cout << "Subscriber initial peers list:" << std::endl;
        for (auto peer : initial_peers)
        {
            std::cout << "   - " << peer << std::endl;
        }
    }

    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Participant_sub");
    auto factory = DomainParticipantFactory::get_instance();

    if (use_env)
    {
        factory->load_profiles();
        factory->get_default_participant_qos(pqos);
    }

    switch (discovery_mode_)
    {
        case eprosima::examples::helloworld::AutomaticDiscovery::OFF:
        {
            // Clear multicast listening locators
            pqos.wire_protocol().builtin.metatrafficMulticastLocatorList.clear();

            // Clear unicast listening locators and add UDPv4 any, letting Fast DDS take care of the
            // port number
            pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.clear();
            eprosima::fastrtps::rtps::Locator_t loc;
            std::istringstream("UDPv4:[0.0.0.0]:0") >> loc;
            pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(loc);
            break;
        }
        case eprosima::examples::helloworld::AutomaticDiscovery::LOCALHOST:
        {
            // Create a descriptor for the new transport.
            auto udp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
            udp_transport->TTL = 0;
            pqos.transport().clear();
            pqos.transport().user_transports.push_back(udp_transport);
            pqos.transport().use_builtin_transports = false;
            break;
        }
        case eprosima::examples::helloworld::AutomaticDiscovery::SUBNET:
        {
            break;
        }
    }

    // Add static initial peers (if any)
    for (auto locator : initial_peers)
    {
        pqos.wire_protocol().builtin.initialPeersList.push_back(locator);
    }

    // Add Host name to participant data
    pqos.user_data().clear();
    pqos.user_data().resize(host_name_.length());
    for (size_t i = 0; i < host_name_.length(); i++)
    {
        pqos.user_data().at(i) = (unsigned char)host_name_.at(i);
    }

    participant_ = factory->create_participant(0, pqos, this, eprosima::fastdds::dds::StatusMask::none());

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    SubscriberQos sqos = SUBSCRIBER_QOS_DEFAULT;

    if (use_env)
    {
        participant_->get_default_subscriber_qos(sqos);
    }

    subscriber_ = participant_->create_subscriber(sqos, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    TopicQos tqos = TOPIC_QOS_DEFAULT;

    if (use_env)
    {
        participant_->get_default_topic_qos(tqos);
    }

    topic_ = participant_->create_topic(
        "HelloWorldTopic",
        "HelloWorld",
        tqos);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    if (use_env)
    {
        subscriber_->get_default_datareader_qos(rqos);
    }

    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" << std::endl;
        }
    }
}

void HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void HelloWorldSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << "samples have been received" << std::endl;
    while (number > listener_.samples_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void HelloWorldSubscriber::on_participant_discovery(
        eprosima::fastdds::dds::DomainParticipant* participant,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    static_cast<void>(participant);
    // New participant discovered
    if (eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT == info.status)
    {
        // Get hostname from user data
        std::string host_name = "";
        unsigned char c = '0';
        for (long unsigned int i = 0; i < info.info.m_userData.size() && c != '\0'; i++)
        {
            c = info.info.m_userData.at(i);
            host_name = host_name + (char)c;
        }

        if (host_name_ == host_name)
        {
            std::cout << "Participant discovered in same host: " << std::endl;
            std::cout << "   - Status: Accepted" << std::endl;
        }
        else
        {
            std::cout << "Participant discovered in different host: " << std::endl;
            if (discovery_mode_ == eprosima::examples::helloworld::AutomaticDiscovery::LOCALHOST)
            {
                participant->ignore_participant(info.info.m_guid);
                std::cout << "   - Status: Ignored" << std::endl;
            }
        }
        std::cout << "   - Name: " << info.info.m_participantName << std::endl;
        std::cout << "   - GUID: " << info.info.m_guid << std::endl;
    }
}