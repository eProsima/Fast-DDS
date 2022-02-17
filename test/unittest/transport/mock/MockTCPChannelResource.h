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

#ifndef MOCK_TCP_CHANNEL_RESOURCE_H
#define MOCK_TCP_CHANNEL_RESOURCE_H

#include <asio.hpp>
#include <fastdds/rtps/transport/TCPChannelResource.h>
#include <fastdds/rtps/transport/TCPTransportInterface.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class MockTCPChannelResource;

using TCPChannelResource = eprosima::fastdds::rtps::TCPChannelResource;
using TCPTransportDescriptor = eprosima::fastdds::rtps::TCPTransportDescriptor;
using TCPTransportInterface = eprosima::fastdds::rtps::TCPTransportInterface;

class MockTCPChannelResource : public TCPChannelResource
{
public:

    MockTCPChannelResource(
            TCPTransportInterface* parent,
            const Locator_t& locator,
            uint32_t maxMsgSize);

    void connect(
            const std::shared_ptr<TCPChannelResource>& myself) override;

    void disconnect() override;

    uint32_t read(
            octet* buffer,
            std::size_t size,
            asio::error_code& ec) override;

    size_t send(
            const octet* header,
            size_t header_size,
            const octet* buffer,
            size_t size,
            asio::error_code& ec) override;

    asio::ip::tcp::endpoint remote_endpoint() const override;

    asio::ip::tcp::endpoint local_endpoint() const override;

    void set_options(
            const TCPTransportDescriptor* options) override;

    void cancel() override;

    void close() override;

    void shutdown(
            asio::socket_base::shutdown_type what) override;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif //MOCK_TCP_CHANNEL_RESOURCE_H
