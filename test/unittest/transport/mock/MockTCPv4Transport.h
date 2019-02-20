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

#include <fastrtps/transport/TCPv4Transport.h>
#include <fastrtps/utils/IPLocator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class MockTCPv4Transport : public TCPv4Transport
{
    public:

    MockTCPv4Transport(const TCPv4TransportDescriptor& descriptor)
    {
        mConfiguration_ = descriptor;
    }

    virtual bool OpenOutputChannel(const Locator_t& locator) override
    {
        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
        TCPChannelResource *channel = new TCPChannelResource(this, nullptr, mService, physicalLocator, 0);
        mChannelResources[physicalLocator] = channel;
        return true;
    }

    virtual bool CloseOutputChannel(const Locator_t& locator) override
    {
        const Locator_t& physicalLocator = IPLocator::toPhysicalLocator(locator);
        auto it = mChannelResources.find(physicalLocator);
        if (it != mChannelResources.end())
        {
            delete it->second;
            mChannelResources.erase(it);
        }
        return true;
    }
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif //MOCK_TRANSPORT_TCP4_STUFF_H