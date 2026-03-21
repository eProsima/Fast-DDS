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

#include "MockReceiverResource.h"

namespace eprosima {
namespace fastdds {
namespace rtps {

MockReceiverResource::MockReceiverResource(
        eprosima::fastdds::rtps::TransportInterface& transport,
        const Locator_t& locator)
    : msg_receiver(nullptr)
{
    m_maxMsgSize = 0x8FFF;
    // Internal channel is opened and assigned to this resource.
    mValid = transport.OpenInputChannel(locator, this, m_maxMsgSize);
    if (!mValid)
    {
        return; // Invalid resource to be discarded by the factory.
    }

    // Implementation functions are bound to the right transport parameters
    Cleanup = [&transport, locator]()
            {
                transport.CloseInputChannel(locator);
            };
    LocatorMapsToManagedChannel = [&transport, locator](const Locator_t& locatorToCheck) -> bool
            {
                return transport.DoInputLocatorsMatch(locator, locatorToCheck);
            };
}

MockReceiverResource::~MockReceiverResource()
{
    if (Cleanup != nullptr)
    {
        Cleanup();
    }

    delete msg_receiver;
}

MessageReceiver* MockReceiverResource::CreateMessageReceiver()
{
    if (msg_receiver == nullptr)
    {
        msg_receiver = new MockMessageReceiver();
        msg_receiver->init(1024);
    }
    return msg_receiver;
}

void MockReceiverResource::OnDataReceived(
        const octet* buf,
        const uint32_t size,
        const Locator_t&,
        const Locator_t& remote)
{
    if (msg_receiver != nullptr)
    {
        CDRMessage_t msg(0);
        msg.wraps = true;
        msg.buffer = const_cast<octet*>(buf);
        msg.length = size;
        msg_receiver->processCDRMsg(remote, &msg);
    }
}

void MockMessageReceiver::setCallback(
        std::function<void()> cb)
{
    this->callback = cb;
}

void MockMessageReceiver::processCDRMsg(
        const Locator_t&,
        CDRMessage_t* msg)
{
    data = msg->buffer;
    if (callback != nullptr)
    {
        callback();
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
