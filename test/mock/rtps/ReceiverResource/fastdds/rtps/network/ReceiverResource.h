// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_RTPS_RECEIVER_RESOURCE_H
#define _FASTDDS_RTPS_RECEIVER_RESOURCE_H

#include <functional>
#include <vector>
#include <memory>
#include <fastrtps/transport/TransportInterface.h>
#include <fastrtps/rtps/Endpoint.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSWriter;
class RTPSReader;
class MessageReceiver;
class RTPSParticipantImpl;

/**
 * Mock ReceiverResource
 * @ingroup NETWORK_MODULE
 */
class ReceiverResource : public TransportReceiverInterface
{
    friend class NetworkFactory;

public:
    bool SupportsLocator(const Locator_t& localLocator)
    {
        if (LocatorMapsToManagedChannel)
        {
            return LocatorMapsToManagedChannel(localLocator);
        }
        return false;
    }
    void Abort() {}
    ReceiverResource(ReceiverResource&&) {}
    virtual ~ReceiverResource() override
    {
        if (Cleanup)
        {
            Cleanup();
        }
    }
    virtual void OnDataReceived(const octet*, const uint32_t,
        const Locator_t&, const Locator_t&) override
    { }

    virtual MessageReceiver* CreateMessageReceiver() { return nullptr; }
    void associateEndpoint(Endpoint *) {}
    void removeEndpoint(Endpoint *) {}
    bool checkReaders(EntityId_t) { return false; }
protected:
    ReceiverResource(TransportInterface& transport,
        const Locator_t& locator)
        : mValid(false)
        , m_maxMsgSize(transport.max_recv_buffer_size())
    {
        mValid = transport.OpenInputChannel(locator, this, m_maxMsgSize);
        if (!mValid)
        {
            return;
        }
        Cleanup = [&transport, locator]() { transport.CloseInputChannel(locator); };
        LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
        { return transport.DoInputLocatorsMatch(locator, locatorToCheck); };
    }

    ReceiverResource() {}
    std::function<void()> Cleanup;
    std::function<bool(const Locator_t&)> LocatorMapsToManagedChannel;
    bool mValid;
    uint32_t m_maxMsgSize;

private:
    ReceiverResource(const ReceiverResource&) = delete;
    ReceiverResource& operator=(const ReceiverResource&) = delete;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
