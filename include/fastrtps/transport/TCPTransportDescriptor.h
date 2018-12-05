// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TCP_TRANSPORT_DESCRIPTOR_H
#define TCP_TRANSPORT_DESCRIPTOR_H

#include "./SocketTransportDescriptor.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
* Transport configuration
* @ingroup TRANSPORT_MODULE
*/
typedef struct TCPTransportDescriptor : public SocketTransportDescriptor {
    virtual ~TCPTransportDescriptor() {}

    std::vector<uint16_t> listening_ports;
    uint32_t keep_alive_frequency_ms;
    uint32_t keep_alive_timeout_ms;
    uint16_t max_logical_port;
    uint16_t logical_port_range;
    uint16_t logical_port_increment;
    uint32_t tcp_negotiation_timeout;
    bool wait_for_tcp_negotiation;

    void add_listener_port(uint16_t port)
    {
        listening_ports.push_back(port);
    }

    RTPS_DllAPI TCPTransportDescriptor();

    RTPS_DllAPI TCPTransportDescriptor(const TCPTransportDescriptor& t);
} TCPTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // TCP_TRANSPORT_DESCRIPTOR_H
