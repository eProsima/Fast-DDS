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

#ifndef MOCK_TRANSPORT_TCP4_STUFF_H
#define MOCK_TRANSPORT_TCP4_STUFF_H

#include <vector>

#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>

#include <rtps/transport/TCPv4Transport.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class MockTCPv4Transport : public TCPv4Transport
{
public:

    MockTCPv4Transport(
            const TCPv4TransportDescriptor& descriptor)
        : TCPv4Transport(descriptor)
    {
    }

    const std::map<Locator_t, std::shared_ptr<TCPChannelResource>>& get_channel_resources() const
    {
        return channel_resources_;
    }

    const std::vector<std::shared_ptr<TCPChannelResource>> get_unbound_channel_resources() const
    {
        return unbound_channel_resources_;
    }

    const std::vector<asio::ip::address_v4>& get_interface_whitelist() const
    {
        return interface_whitelist_;
    }

    const std::map<Locator_t, std::shared_ptr<fastdds::rtps::TCPAcceptor>>& get_acceptors_map() const
    {
        return acceptors_;
    }

    bool send(
            const fastdds::rtps::octet* send_buffer,
            uint32_t send_buffer_size,
            const Locator_t& send_resource_locator,
            const Locator_t& remote_locator)
    {
        eprosima::fastdds::rtps::NetworkBuffer buffers(send_buffer, send_buffer_size);
        std::vector<eprosima::fastdds::rtps::NetworkBuffer> buffer_list;
        buffer_list.push_back(buffers);
        return TCPv4Transport::send(buffer_list, send_buffer_size, send_resource_locator, remote_locator);
    }

    const std::map<Locator_t, std::set<uint16_t>>& get_channel_pending_logical_ports() const
    {
        return channel_pending_logical_ports_;
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //MOCK_TRANSPORT_TCP4_STUFF_H
