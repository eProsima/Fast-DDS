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

#ifndef MOCK_CLI_MANAGER_H
#define MOCK_CLI_MANAGER_H

#include "CliDiscoveryManager.hpp"
#include "CliDiscoveryParser.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <gmock/gmock.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::option;

namespace eprosima {
namespace fastdds {
namespace dds {

class MockCliDiscoveryManager : public CliDiscoveryManager
{
public:

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

};
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // MOCK_CLI_MANAGER_H