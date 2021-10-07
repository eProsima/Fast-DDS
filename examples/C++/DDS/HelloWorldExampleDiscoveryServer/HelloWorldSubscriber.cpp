// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <condition_variable>
#include <csignal>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

#include "HelloWorldSubscriber.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

using IPLocator = eprosima::fastrtps::rtps::IPLocator;

std::atomic<bool> HelloWorldSubscriber::stop_(false);
std::mutex HelloWorldSubscriber::terminate_cv_mtx_;
std::condition_variable HelloWorldSubscriber::terminate_cv_;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldSubscriber::is_stopped()
{
    return stop_;
}

void HelloWorldSubscriber::stop()
{
    stop_ = true;
    terminate_cv_.notify_all();
}

bool HelloWorldSubscriber::init(
        const std::string& topic_name,
        uint32_t max_messages,
        bool discovery_server,
        eprosima::fastdds::rtps::Locator discovery_server_address,
        bool discovery_client,
        eprosima::fastdds::rtps::Locator discovery_remote_address,
        bool tcp_server,
        const std::string& tcp_server_ip,
        unsigned short tcp_server_port,
        bool tcp_client,
        const std::string& tcp_remote_ip,
        unsigned short tcp_remote_port)
{
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");

    if (tcp_server || tcp_client)
    {
        int32_t kind = LOCATOR_KIND_TCPv4;
        Locator initial_peer_locator;
        initial_peer_locator.kind = kind;

        pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
        pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod =
                eprosima::fastrtps::Duration_t(5, 0);

        pqos.transport().use_builtin_transports = false;
        std::shared_ptr<TCPv4TransportDescriptor> tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();

        // if (tcp_client && !tcp_remote_ip.empty())
        // {
        //     IPLocator::setIPv4(initial_peer_locator, tcp_remote_ip);
        //     initial_peer_locator.port = tcp_remote_port;
        //     pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer_locator);
        //     std::cout << "Remote TCP server address -> " << tcp_remote_ip << ":" << tcp_remote_port << std::endl;
        // }

        if (tcp_server)
        {
            tcp_descriptor->sendBufferSize = 0;
            tcp_descriptor->receiveBufferSize = 0;
        }

        if (tcp_server && !tcp_server_ip.empty())
        {
            tcp_descriptor->set_WAN_address(tcp_server_ip);
            tcp_descriptor->add_listener_port(tcp_server_port);
            std::cout << "TCP server address -> " << tcp_server_ip << ":" << tcp_server_port << std::endl;
        }
        pqos.transport().user_transports.push_back(tcp_descriptor);
    }

    // Set participant as DS SERVER
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol_t::SERVER;

    // Set SERVER's GUID prefix
    std::istringstream("44.53.22.5f.45.50.52.4f.53.49.4d.41") >> pqos.wire_protocol().prefix;

    if (discovery_server)
    {
        if (tcp_server || tcp_client)
        {
            discovery_server_address.kind = LOCATOR_KIND_TCPv4;
        }

        // Set SERVER's listening locator for PDP
        pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(discovery_server_address);

        std::cout << "Discovery Server address -> " << IPLocator::toIPv4string(discovery_server_address) <<
                ":" << discovery_server_address.port << std::endl;
    }

    if (discovery_client)
    {
        if (tcp_server || tcp_client)
        {
            discovery_remote_address.kind = LOCATOR_KIND_TCPv4;
        }

        // Set remote SERVER's GUID prefix
        RemoteServerAttributes remote_server_att;
        remote_server_att.ReadguidPrefix("44.53.11.5f.45.50.52.4f.53.49.4d.41");

        // Set SERVER's listening locator for PDP
        remote_server_att.metatrafficUnicastLocatorList.push_back(discovery_remote_address);

        // Add remote SERVER to CLIENT's list of SERVERs
        pqos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(remote_server_att);

        std::cout << "Remote Discovery Server address -> " << IPLocator::toIPv4string(discovery_remote_address) <<
                ":" << discovery_remote_address.port << std::endl;
    }

    // if (tcp_server || tcp_client)
    // {
    //     int32_t kind = LOCATOR_KIND_TCPv4;
    //     Locator initial_peer_locator;
    //     initial_peer_locator.kind = kind;

    //     pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    //     pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod =
    //             eprosima::fastrtps::Duration_t(5, 0);

    //     pqos.transport().use_builtin_transports = false;
    //     std::shared_ptr<TCPv4TransportDescriptor> tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();

    //     // if (tcp_client && !tcp_remote_ip.empty())
    //     // {
    //     //     IPLocator::setIPv4(initial_peer_locator, tcp_remote_ip);
    //     //     initial_peer_locator.port = tcp_remote_port;
    //     //     pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer_locator);
    //     //     std::cout << "Remote TCP server address -> " << tcp_remote_ip << ":" << tcp_remote_port << std::endl;
    //     // }

    //     if (tcp_server)
    //     {
    //         tcp_descriptor->sendBufferSize = 0;
    //         tcp_descriptor->receiveBufferSize = 0;
    //     }

    //     if (tcp_server && !tcp_server_ip.empty())
    //     {
    //         tcp_descriptor->set_WAN_address(tcp_server_ip);
    //         tcp_descriptor->add_listener_port(tcp_server_port);
    //         std::cout << "TCP server address -> " << tcp_server_ip << ":" << tcp_server_port << std::endl;
    //     }
    //     pqos.transport().user_transports.push_back(tcp_descriptor);
    // }

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    // CREATE THE TOPIC
    topic_ = participant_->create_topic(
        topic_name,
        "HelloWorld",
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE READER
    if (max_messages > 0)
    {
        listener_.set_max_messages(max_messages);
    }
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    if (participant_ != nullptr)
    {
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            if (reader_ != nullptr)
            {
                subscriber_->delete_datareader(reader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void HelloWorldSubscriber::SubListener::set_max_messages(
        uint32_t max_messages)
{
    max_messages_ = max_messages;
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
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
    while ((reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" << std::endl;
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void HelloWorldSubscriber::run(
        uint32_t samples)
{
    stop_ = false;
    if (samples > 0)
    {
        std::cout << "Subscriber running until " << samples <<
            " samples have been received. Please press CTRL+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Subscriber running. Please press CTRL+C to stop the Subscriber." << std::endl;
    }
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Subscriber execution." << std::endl;
                static_cast<void>(signum); HelloWorldSubscriber::stop();
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
