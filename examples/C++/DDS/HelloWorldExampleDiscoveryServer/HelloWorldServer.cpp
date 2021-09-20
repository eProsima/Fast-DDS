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

#include "HelloWorldServer.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>

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
        eprosima::fastdds::rtps::Locator server_address)
{
    DomainParticipantQos pqos;
    pqos.name("Participant_server");

    // Set participant as SERVER
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol_t::SERVER;

    // Set SERVER's GUID prefix
    std::istringstream("44.53.00.5f.45.50.52.4f.53.49.4d.41") >> pqos.wire_protocol().prefix;

    // Set SERVER's listening locator for PDP
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(server_address);

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldServer::~HelloWorldServer()
{
    if (participant_ != nullptr)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void HelloWorldServer::run()
{
    stop_ = false;
    std::cout << "Server running. Please press CTRL+C to stop the Server" << std::endl;
    signal(SIGINT, [](int signum)
            {
                static_cast<void>(signum); HelloWorldServer::stop();
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
