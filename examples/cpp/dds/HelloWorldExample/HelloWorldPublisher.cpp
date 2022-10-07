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
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"

#include <string>
#include <thread>

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include "common.hpp"

using namespace eprosima::fastdds::dds;

HelloWorldPublisher::HelloWorldPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , host_name_(asio::ip::host_name())
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldPublisher::init(
        bool use_env,
        eprosima::examples::helloworld::AutomaticDiscovery discovery_mode,
        const eprosima::fastdds::rtps::LocatorList& initial_peers)
{
    discovery_mode_ = discovery_mode;
    std::cout << "Publisher discovery mode: " << discovery_mode_ << std::endl;
    if (!initial_peers.empty())
    {
        std::cout << "Publisher initial peers list:" << std::endl;
        for (auto peer : initial_peers)
        {
            std::cout << "   - " << peer << std::endl;
        }
    }

    hello_.index(0);
    hello_.message("HelloWorld");
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Participant_pub");
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

    participant_ = factory->create_participant(0, pqos, this);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE PUBLISHER
    PublisherQos pubqos = PUBLISHER_QOS_DEFAULT;

    if (use_env)
    {
        participant_->get_default_publisher_qos(pubqos);
    }

    publisher_ = participant_->create_publisher(
        pubqos,
        nullptr);

    if (publisher_ == nullptr)
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

    // CREATE THE WRITER
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;

    if (use_env)
    {
        publisher_->get_default_datawriter_qos(wqos);
    }

    writer_ = publisher_->create_datawriter(
        topic_,
        wqos,
        &listener_);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }
    if (publisher_ != nullptr)
    {
        participant_->delete_publisher(publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        firstConnected_ = true;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    if (samples == 0)
    {
        while (!stop_)
        {
            if (publish(false))
            {
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
    else
    {
        for (uint32_t i = 0; i < samples; ++i)
        {
            if (!publish())
            {
                --i;
            }
            else
            {
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
}

void HelloWorldPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop_ = false;
    std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop_ = true;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

bool HelloWorldPublisher::publish(
        bool waitForListener)
{
    if (listener_.firstConnected_ || !waitForListener || listener_.matched_ > 0)
    {
        hello_.index(hello_.index() + 1);
        writer_->write(&hello_);
        return true;
    }
    return false;
}

void HelloWorldPublisher::on_participant_discovery(
        eprosima::fastdds::dds::DomainParticipant* participant,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
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
