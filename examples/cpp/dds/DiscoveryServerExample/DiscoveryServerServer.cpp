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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
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
        TransportKind transport)
{
    DomainParticipantQos pqos;
    pqos.name("DS-Server");
    pqos.transport().use_builtin_transports = false;

    // Create DS SERVER locator
    eprosima::fastdds::rtps::Locator server_locator;
    eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(server_locator, server_port);

    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> descriptor;

    switch (transport)
    {
    case TransportKind::SHM:
        descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
        server_locator.kind = LOCATOR_KIND_SHM;
        break;

    case TransportKind::UDPv4:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        descriptor_tmp->interfaceWhiteList.push_back(server_address);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_UDPv4;
        eprosima::fastrtps::rtps::IPLocator::setIPv4(server_locator, server_address);
        break;
    }

    case TransportKind::UDPv6:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
        descriptor_tmp->interfaceWhiteList.push_back(server_address);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_UDPv6;
        eprosima::fastrtps::rtps::IPLocator::setIPv6(server_locator, server_address);
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
    std::istringstream(DEFAULT_ROS2_SERVER_GUIDPREFIX) >> pqos.wire_protocol().prefix;

    // Set SERVER's listening locator for PDP
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(server_locator);

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos, &listener_);

    if (participant_ == nullptr)
    {
        return false;
    }

    std::cout <<
        "Server Participant " << pqos.name() <<
        " created with GUID " << participant_->guid() <<
        " listening in address <" << server_locator  << "> " <<
        std::endl;

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

void DiscoveryServer::run()
{
    stop_ = false;
    std::cout << "Server running. Please press CTRL+C to stop the Server." << std::endl;
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Server execution." << std::endl;
                static_cast<void>(signum); DiscoveryServer::stop();
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
