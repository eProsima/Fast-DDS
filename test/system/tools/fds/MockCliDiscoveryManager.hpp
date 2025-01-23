// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef MOCK_CLI_MANAGER_H
#define MOCK_CLI_MANAGER_H

#include "CliDiscoveryManager.hpp"
#include "CliDiscoveryParser.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::option;

namespace eprosima {
namespace fastdds {
namespace dds {

class MockCliDiscoveryManager : public CliDiscoveryManager
{
public:

#ifndef _WIN32
    std::vector<uint16_t> get_listening_ports() override
    {
        return mocked_ports;
    }

    std::vector<uint16_t> real_get_listening_ports()
    {
        return CliDiscoveryManager::get_listening_ports();
    }

#endif // _WIN32

    DomainParticipantQos getServerQos()
    {
        return serverQos;
    }

    DomainParticipant* getServer()
    {
        return pServer;
    }

    std::list<uint16_t> getUdpPorts()
    {
        return udp_ports_;
    }

    std::list<std::string> getUdpIps()
    {
        return udp_ips_;
    }

    std::list<uint16_t> getTcpPorts()
    {
        return tcp_ports_;
    }

    std::list<std::string> getTcpIps()
    {
        return tcp_ips_;
    }

    void reset()
    {
        udp_ports_.clear();
        udp_ips_.clear();
        tcp_ports_.clear();
        tcp_ips_.clear();
        serverQos = DomainParticipantQos();
    }

    std::vector<uint16_t> mocked_ports;
};
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // MOCK_CLI_MANAGER_H
