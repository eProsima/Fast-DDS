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
 * @file BigDataTransportDiscServer.cpp
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

#include "BigDataTransportDiscServer.h"

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
        const std::string& profile)
{
    ///////////////////////////////
    // Create Participant
    ///////////////////////////////

    // CREATE THE PARTICIPANT
    std::cout << "Creating Server participant from XML profile" << std::endl;
    participant_ = DomainParticipantFactory::get_instance()->create_participant_with_profile(profile);

    if (participant_ == nullptr)
    {
        return false;
    }

    std::cout <<
        "Server Participant created with GUID " << participant_->guid() <<
        " listening in address <" << participant_->get_listener()  << "> " <<
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
