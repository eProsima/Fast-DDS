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

#include "HelloWorldServer.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <condition_variable>
#include <mutex>
#include <csignal>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

namespace server_ns
{
    bool stop;
    std::mutex mtx;
    std::condition_variable terminate_cv;
}

using namespace server_ns;

HelloWorldServer::HelloWorldServer()
    : participant_(nullptr)
{
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

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldServer::~HelloWorldServer()
{
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldServer::run()
{
    std::cout << "Server running. Please press CTRL+C to stop the Server at any time." << std::endl;
    stop = false;
    signal(SIGINT, [](int signum){static_cast<void>(signum); stop=true; terminate_cv.notify_one();});
    std::unique_lock<std::mutex> lck(mtx);
    terminate_cv.wait(lck, []{return stop;});
}
