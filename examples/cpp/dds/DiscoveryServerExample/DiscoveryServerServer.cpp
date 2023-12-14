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
 * @file DiscoveryServerServer.cpp
 *
 */

#include <condition_variable>
#include <csignal>
#include <mutex>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include "DiscoveryServerServer.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> DiscoveryServer::stop_(false);
std::mutex DiscoveryServer::terminate_cv_mtx_;
std::condition_variable DiscoveryServer::terminate_cv_;

DiscoveryServer::DiscoveryServer()
    : participant_(nullptr)
{
}

bool DiscoveryServer::is_stopped()
{
    return stop_;
}

void DiscoveryServer::stop()
{
    stop_ = true;
    terminate_cv_.notify_all();
}

bool DiscoveryServer::init(
        const std::string& server_address,
        unsigned short server_port,
        unsigned short server_id,
        TransportKind transport,
        bool has_connection_server,
        const std::string& connection_server_address,
        unsigned short connection_server_port,
        unsigned short connection_server_id)
{
    DomainParticipantQos pqos;
    pqos.name("DS-Server");
    pqos.transport().use_builtin_transports = false;

    std::string ip_listening_address(server_address);
    std::string ip_connection_address(connection_server_address);
    // Check if DNS is required
    if (!is_ip(server_address))
    {
        ip_listening_address = get_ip_from_dns(server_address, transport);
    }

    if (ip_listening_address.empty())
    {
        return false;
    }

    // Do the same for connection
    if (has_connection_server && !is_ip(connection_server_address))
    {
        ip_connection_address = get_ip_from_dns(connection_server_address, transport);
    }

    if (has_connection_server && ip_connection_address.empty())
    {
        return false;
    }

    ///////////////////////////////
    // Configure Listening address
    ///////////////////////////////

    // Create DS SERVER locator
    eprosima::fastdds::rtps::Locator listening_locator;
    eprosima::fastdds::rtps::Locator connection_locator;
    eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(listening_locator, server_port);
    eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(connection_locator, connection_server_port);

    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> descriptor;

    switch (transport)
    {
        case TransportKind::SHM:
            descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
            listening_locator.kind = LOCATOR_KIND_SHM;
            connection_locator.kind = LOCATOR_KIND_SHM;
            break;

        case TransportKind::UDPv4:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
            // descriptor_tmp->interfaceWhiteList.push_back(ip_listening_address);
            descriptor = descriptor_tmp;

            listening_locator.kind = LOCATOR_KIND_UDPv4;
            eprosima::fastrtps::rtps::IPLocator::setIPv4(listening_locator, ip_listening_address);
            connection_locator.kind = LOCATOR_KIND_UDPv4;
            eprosima::fastrtps::rtps::IPLocator::setIPv4(connection_locator, ip_connection_address);
            break;
        }

        case TransportKind::UDPv6:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
            // descriptor_tmp->interfaceWhiteList.push_back(ip_listening_address);
            descriptor = descriptor_tmp;

            listening_locator.kind = LOCATOR_KIND_UDPv6;
            eprosima::fastrtps::rtps::IPLocator::setIPv6(listening_locator, ip_listening_address);
            connection_locator.kind = LOCATOR_KIND_UDPv6;
            eprosima::fastrtps::rtps::IPLocator::setIPv6(connection_locator, ip_connection_address);
            break;
        }

        case TransportKind::TCPv4:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
            // descriptor_tmp->interfaceWhiteList.push_back(ip_listening_address);
            descriptor_tmp->add_listener_port(server_port);
            descriptor = descriptor_tmp;

            listening_locator.kind = LOCATOR_KIND_TCPv4;
            eprosima::fastrtps::rtps::IPLocator::setLogicalPort(listening_locator, server_port);
            eprosima::fastrtps::rtps::IPLocator::setIPv4(listening_locator, ip_listening_address);
            connection_locator.kind = LOCATOR_KIND_TCPv4;
            eprosima::fastrtps::rtps::IPLocator::setIPv4(connection_locator, ip_connection_address);
            eprosima::fastrtps::rtps::IPLocator::setLogicalPort(connection_locator, connection_server_port);
            break;
        }

        case TransportKind::TCPv6:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
            // descriptor_tmp->interfaceWhiteList.push_back(ip_listening_address);
            descriptor_tmp->add_listener_port(server_port);
            descriptor = descriptor_tmp;

            listening_locator.kind = LOCATOR_KIND_TCPv6;
            eprosima::fastrtps::rtps::IPLocator::setLogicalPort(listening_locator, server_port);
            eprosima::fastrtps::rtps::IPLocator::setIPv6(listening_locator, ip_listening_address);
            connection_locator.kind = LOCATOR_KIND_TCPv6;
            eprosima::fastrtps::rtps::IPLocator::setIPv6(connection_locator, ip_connection_address);
            eprosima::fastrtps::rtps::IPLocator::setLogicalPort(connection_locator, connection_server_port);
            break;
        }

        default:
            break;
    }

    // Add descriptor
    pqos.transport().user_transports.push_back(descriptor);

    // Set participant as SERVER
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol_t::SERVER;

    // Set SERVER's GUID prefix
    pqos.wire_protocol().prefix = get_discovery_server_guid_from_id(server_id);

    // Set SERVER's listening locator for PDP
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(listening_locator);

    ///////////////////////////////
    // Configure Connection address
    ///////////////////////////////

    RemoteServerAttributes remote_server_att;
    if (has_connection_server)
    {
        // Set SERVER's GUID prefix
        remote_server_att.guidPrefix = get_discovery_server_guid_from_id(connection_server_id);

        // Set SERVER's listening locator for PDP
        remote_server_att.metatrafficUnicastLocatorList.push_back(connection_locator);

        // Add remote SERVER to CLIENT's list of SERVERs
        pqos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(remote_server_att);
    }


    ///////////////////////////////
    // Create Participant
    ///////////////////////////////

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos, &listener_);

    if (participant_ == nullptr)
    {
        return false;
    }


    if (has_connection_server)
    {
        std::cout <<
            "Server Participant " << pqos.name() <<
            " created with GUID " << participant_->guid() <<
            " listening in address <" << listening_locator  << "> " <<
            " connecting with Discovery Server <" << remote_server_att.guidPrefix << "> "
            " with address <" << connection_locator  << "> " <<
            std::endl;
    }
    else
    {
        std::cout <<
            "Server Participant " << pqos.name() <<
            " created with GUID " << participant_->guid() <<
            " listening in address <" << listening_locator  << "> " <<
            std::endl;
    }

    return true;
}

DiscoveryServer::~DiscoveryServer()
{
    if (participant_ != nullptr)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void DiscoveryServer::ServerListener::on_participant_discovery(
        eprosima::fastdds::dds::DomainParticipant* /*participant*/,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Discovered Participant with GUID " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT ||
            info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Dropped Participant with GUID " << info.info.m_guid << std::endl;
    }
}

void DiscoveryServer::run(
        unsigned int timeout)
{
    stop_ = false;
    std::cout << "Server running. Please press CTRL+C to stop the Server." << std::endl;
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Server execution." << std::endl;
                static_cast<void>(signum); DiscoveryServer::stop();
            });

    if (timeout > 0)
    {
        // Create a thread that will stop this process after timeout
        std::thread t(
            [=]
                ()
            {
                std::this_thread::sleep_for(std::chrono::seconds(timeout));
                std::cout << "Stopping Server execution due to timeout." << std::endl;
                DiscoveryServer::stop();
            });
        t.detach();
    }

    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
