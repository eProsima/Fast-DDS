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
namespace fastrtps {
namespace rtps {

TCPChannelResourceMock::TCPChannelResourceMock(
        TCPTransportInterface* parent,
        const Locator_t& locator,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, locator, maxMsgSize)
{
}

void TCPChannelResourceMock::connect(
        const std::shared_ptr<TCPChannelResource>&)
{
    connection_status_.exchange(eConnectionStatus::eConnecting);
}

void TCPChannelResourceMock::disconnect()
{
    connection_status_.exchange(eConnectionStatus::eDisconnected);
}

uint32_t TCPChannelResourceMock::read(
        octet*,
        std::size_t,
        asio::error_code&)
{
    return 0;
}

size_t TCPChannelResourceMock::send(
        const octet*,
        size_t,
        const octet*,
        size_t,
        asio::error_code&)
{
    return 0;
}

asio::ip::tcp::endpoint TCPChannelResourceMock::remote_endpoint() const
{
    asio::ip::tcp::endpoint ep;
    return ep;
}

asio::ip::tcp::endpoint TCPChannelResourceMock::local_endpoint() const
{
    asio::ip::tcp::endpoint ep;
    return ep;
}

void TCPChannelResourceMock::set_options(
        const TCPTransportDescriptor*)
{
}

void TCPChannelResourceMock::cancel()
{
}

void TCPChannelResourceMock::close()
{
}

void TCPChannelResourceMock::shutdown(
        asio::socket_base::shutdown_type)
{
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
