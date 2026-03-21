// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "MockTCPChannelResource.h"

namespace eprosima {
namespace fastdds {
namespace rtps {

MockTCPChannelResource::MockTCPChannelResource(
        TCPTransportInterface* parent,
        const Locator_t& locator,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, locator, maxMsgSize)
{
}

void MockTCPChannelResource::connect(
        const std::shared_ptr<TCPChannelResource>&)
{
    connection_status_.exchange(eConnectionStatus::eConnecting);
}

void MockTCPChannelResource::disconnect()
{
    connection_status_.exchange(eConnectionStatus::eDisconnected);
}

uint32_t MockTCPChannelResource::read(
        octet*,
        std::size_t,
        asio::error_code&)
{
    return 0;
}

size_t MockTCPChannelResource::send(
        const octet*,
        size_t,
        const octet*,
        size_t,
        asio::error_code&)
{
    return 0;
}

size_t MockTCPChannelResource::send(
        const octet*,
        size_t,
        const std::vector<NetworkBuffer>&,
        uint32_t,
        asio::error_code&)
{
    return 0;
}

asio::ip::tcp::endpoint MockTCPChannelResource::remote_endpoint() const
{
    asio::ip::tcp::endpoint ep;
    return ep;
}

asio::ip::tcp::endpoint MockTCPChannelResource::local_endpoint() const
{
    asio::ip::tcp::endpoint ep;
    return ep;
}

asio::ip::tcp::endpoint MockTCPChannelResource::remote_endpoint(
        asio::error_code& ec) const
{
    ec = asio::error_code();  // Indicate no error
    asio::ip::tcp::endpoint ep;
    return ep;
}

asio::ip::tcp::endpoint MockTCPChannelResource::local_endpoint(
        asio::error_code& ec) const
{
    ec = asio::error_code();  // Indicate no error
    asio::ip::tcp::endpoint ep;
    return ep;
}

void MockTCPChannelResource::set_options(
        const TCPTransportDescriptor*)
{
}

void MockTCPChannelResource::cancel()
{
}

void MockTCPChannelResource::close()
{
}

void MockTCPChannelResource::shutdown(
        asio::socket_base::shutdown_type)
{
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
