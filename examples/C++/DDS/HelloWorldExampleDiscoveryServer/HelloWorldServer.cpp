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
 * @file HelloWorldServer.cpp
 *
 */

#include <condition_variable>
#include <csignal>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/utils/IPLocator.h>

#include "HelloWorldServer.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> HelloWorldServer::stop_(false);
std::mutex HelloWorldServer::terminate_cv_mtx_;
std::condition_variable HelloWorldServer::terminate_cv_;

HelloWorldServer::HelloWorldServer()
    : participant_(nullptr)
{
}

bool HelloWorldServer::is_stopped()
{
    return stop_;
}

void HelloWorldServer::stop()
{
    stop_ = true;
    terminate_cv_.notify_all();
}

bool HelloWorldServer::init(
        const std::string& server_address,
        unsigned short server_port,
        bool tcp)
{
    DomainParticipantQos pqos;
    pqos.name("DS-Server");

    // Create DS SERVER locator
    eprosima::fastdds::rtps::Locator server_locator;
    eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(server_locator, server_port);
    eprosima::fastrtps::rtps::IPLocator::setLogicalPort(server_locator, server_port);
    eprosima::fastrtps::rtps::IPLocator::setIPv4(server_locator, server_address);
    eprosima::fastrtps::rtps::IPLocator::setWan(server_locator, server_address);
    if (tcp)
    {
        server_locator.kind = LOCATOR_KIND_TCPv4;
    }
    else
    {
        server_locator.kind = LOCATOR_KIND_UDPv4;   // default
    }

    // Set participant as SERVER
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol_t::SERVER;

    // Set SERVER's GUID prefix
    std::istringstream(DEFAULT_ROS2_SERVER_GUIDPREFIX) >> pqos.wire_protocol().prefix;

    // Set SERVER's listening locator for PDP
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(server_locator);

    // TCP CONFIGURATION
    if (tcp)
    {
        pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
        pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod =
                eprosima::fastrtps::Duration_t(5, 0);

        pqos.transport().use_builtin_transports = false;
        std::shared_ptr<TCPv4TransportDescriptor> tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();

        tcp_descriptor->sendBufferSize = 0;
        tcp_descriptor->receiveBufferSize = 0;
        tcp_descriptor->set_WAN_address(server_address);
        tcp_descriptor->add_listener_port(server_port);

        pqos.transport().user_transports.push_back(tcp_descriptor);
    }

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos, &listener_);

    if (participant_ == nullptr)
    {
        return false;
    }

    std::cout << "Participant " << pqos.name() << " created with GUID " << participant_->guid() << std::endl;

    return true;
}

HelloWorldServer::~HelloWorldServer()
{
    if (participant_ != nullptr)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void HelloWorldServer::ServerListener::on_participant_discovery(
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

void HelloWorldServer::run()
{
    stop_ = false;
    std::cout << "Server running. Please press CTRL+C to stop the Server." << std::endl;
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Server execution." << std::endl;
                static_cast<void>(signum); HelloWorldServer::stop();
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
