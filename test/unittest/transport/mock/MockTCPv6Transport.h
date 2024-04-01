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

#ifndef MOCK_TRANSPORT_TCP6_STUFF_H
#define MOCK_TRANSPORT_TCP6_STUFF_H

#include <fastrtps/transport/TCPv6TransportDescriptor.h>
#include <rtps/transport/TCPv6Transport.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

using TCPv6Transport = eprosima::fastdds::rtps::TCPv6Transport;
using TCPChannelResource = eprosima::fastdds::rtps::TCPChannelResource;
using TCPChannelResourceBasic = eprosima::fastdds::rtps::TCPChannelResourceBasic;

class MockTCPv6Transport : public TCPv6Transport
{
public:

    MockTCPv6Transport(
            const TCPv6TransportDescriptor& descriptor)
        : TCPv6Transport(descriptor)
    {
    }

    const std::map<Locator_t, std::shared_ptr<TCPChannelResource>>& get_channel_resources() const
    {
        return channel_resources_;
    }

    const std::vector<std::shared_ptr<TCPChannelResource>>& get_unbound_channel_resources() const
    {
        return unbound_channel_resources_;
    }

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif //MOCK_TRANSPORT_TCP6_STUFF_H
