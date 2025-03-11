// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ClientPublisherApp.cpp
 *
 */

#include "ClientPublisherApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace discovery_server {

ClientPublisherApp::ClientPublisherApp(
        const CLIParser::client_publisher_config& config)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new HelloWorldPubSubType())
    , matched_(0)
    , samples_(config.samples)
    , stop_(false)
    , period_ms_(config.interval)
{
    // Set up the data type with initial values
    hello_.index(0);
    hello_.message("Hello world");

    // Configure Participant QoS
    DomainParticipantQos pqos;

    pqos.name("DS-Client_pub");
    pqos.transport().use_builtin_transports = false;

    uint16_t server_port = config.connection_port;

    std::string ip_server_address(config.connection_address);

    // Check if DNS is required
    if (!is_ip(config.connection_address))
    {
        ip_server_address = get_ip_from_dns(config.connection_address, config.transport_kind);
    }

    if (ip_server_address.empty())
    {
        throw std::runtime_error("Invalid connection address");
    }

    // Create DS locator
    eprosima::fastdds::rtps::Locator server_locator;
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(server_locator, server_port);

    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> descriptor;

    switch (config.transport_kind)
    {
        case TransportKind::UDPv4:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
            // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
            descriptor = descriptor_tmp;

            server_locator.kind = LOCATOR_KIND_UDPv4;
            eprosima::fastdds::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
            break;
        }

        case TransportKind::UDPv6:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
            // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
            descriptor = descriptor_tmp;

            server_locator.kind = LOCATOR_KIND_UDPv6;
            eprosima::fastdds::rtps::IPLocator::setIPv6(server_locator, ip_server_address);
            break;
        }

        case TransportKind::TCPv4:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
            descriptor_tmp->add_listener_port(0);
            descriptor = descriptor_tmp;

            server_locator.kind = LOCATOR_KIND_TCPv4;
            eprosima::fastdds::rtps::IPLocator::setLogicalPort(server_locator, server_port);
            eprosima::fastdds::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
            break;
        }

        case TransportKind::TCPv6:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
            descriptor_tmp->add_listener_port(0);
            descriptor = descriptor_tmp;

            server_locator.kind = LOCATOR_KIND_TCPv6;
            eprosima::fastdds::rtps::IPLocator::setLogicalPort(server_locator, server_port);
            eprosima::fastdds::rtps::IPLocator::setIPv6(server_locator, ip_server_address);
            break;
        }

        default:
            break;
    }

    // Set participant as DS CLIENT
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastdds::rtps::DiscoveryProtocol::CLIENT;

    // Add remote SERVER to CLIENT's list of SERVERs
    pqos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(server_locator);

    // Add descriptor
    pqos.transport().user_transports.push_back(descriptor);

    // Create Domainparticipant
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos, nullptr);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    std::cout <<
        "Publisher Participant " << pqos.name() <<
        " created with GUID " << participant_->guid() <<
        " connecting to server <" << server_locator  << "> " <<
        std::endl;

    // Regsiter type
    type_.register_type(participant_);

    // Create the publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the topic
    topic_ = participant_->create_topic(config.topic_name, type_.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create de data writer
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;

    if (!config.reliable)
    {
        wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    if (!config.transient_local)
    {
        wqos.durability().kind = VOLATILE_DURABILITY_QOS;
    }

    // So as not to overwriter the first sample
    // if we publish inmediately after the discovery
    // and the suscription is not prepared yet
    wqos.history().depth = 5;

    writer_ = publisher_->create_datawriter(topic_, wqos, this);

    if (writer_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }
}

ClientPublisherApp::~ClientPublisherApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void ClientPublisherApp::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher matched." << std::endl;
        cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void ClientPublisherApp::run()
{
    while (!is_stopped() && ((samples_ == 0) || (hello_.index() < samples_)))
    {
        if (publish())
        {
            std::cout << "Message: '" << hello_.message() << "' with index: '" << hello_.index()
                      << "' SENT" << std::endl;
        }
        // Wait for period or stop event
        std::unique_lock<std::mutex> period_lock(mutex_);
        cv_.wait_for(period_lock, std::chrono::milliseconds(period_ms_), [&]()
                {
                    return is_stopped();
                });
    }
}

bool ClientPublisherApp::publish()
{
    bool ret = false;
    // Wait for the data endpoints discovery
    std::unique_lock<std::mutex> matched_lock(mutex_);
    cv_.wait(matched_lock, [&]()
            {
                // at least one has been discovered
                return ((matched_ > 0) || is_stopped());
            });

    if (!is_stopped())
    {
        hello_.index(hello_.index() + 1);
        ret = (RETCODE_OK == writer_->write(&hello_));
    }
    return ret;
}

bool ClientPublisherApp::is_stopped()
{
    return stop_.load();
}

void ClientPublisherApp::stop()
{
    stop_.store(true);
    cv_.notify_one();
}

} // namespace discovery_server
} // namespace examples
} // namespace fastdds
} // namespace eprosima
