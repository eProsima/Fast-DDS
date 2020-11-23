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

#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>
#include <rtps/transport/TCPv4Transport.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

using TCPv4Transport = eprosima::fastdds::rtps::TCPv4Transport;
using TCPChannelResource = eprosima::fastdds::rtps::TCPChannelResource;
using TCPChannelResourceBasic = eprosima::fastdds::rtps::TCPChannelResourceBasic;
using TCPChannelResourceSecure = eprosima::fastdds::rtps::TCPChannelResourceSecure;

class MockTCPv4Transport : public TCPv4Transport
{
public:

    MockTCPv4Transport(const TCPv4TransportDescriptor& descriptor)
    {
        configuration_ = descriptor;
    }

    virtual bool OpenOutputChannel(
            SendResourceList&,
            const Locator_t& locator) override
    {
        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
        std::shared_ptr<TCPChannelResource> channel(
#if TLS_FOUND
            (configuration_.apply_security) ?
                static_cast<TCPChannelResource*>(
                    new TCPChannelResourceSecure(this, io_service_, ssl_context_, physicalLocator, 0)) :
#endif
                static_cast<TCPChannelResource*>(
                    new TCPChannelResourceBasic(this, io_service_, physicalLocator, 0))
            );

        channel_resources_[physicalLocator] = channel;
        return true;
    }

        /*
    virtual bool CloseOutputChannel(const Locator_t& locator) override
    {
        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
        auto it = channel_resources_.find(physicalLocator);
        if (it != channel_resources_.end())
        {
            delete it->second;
            channel_resources_.erase(it);
        }
        return true;
    }
        */
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif //MOCK_TRANSPORT_TCP4_STUFF_H
