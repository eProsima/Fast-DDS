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
        TransportInterface& transport,
        const Locator_t& locator)
    : msg_receiver(nullptr)
{
    // Internal channel is opened and assigned to this resource.
    valid_ = transport.OpenInputChannel(locator, this, 0x8FFF);
    if (!valid_)
    {
        return;
    }

    cleanup_ = [&transport, locator]()
            {
                transport.CloseInputChannel(locator);
            };
}

MockReceiverResource::~MockReceiverResource()
{
    if (cleanup_ != nullptr)
    {
        cleanup_();
    }

    delete msg_receiver.load();
}

MockMessageReceiver* MockReceiverResource::CreateMessageReceiver()
{
    if (msg_receiver.load() == nullptr)
    {
        msg_receiver.store(new MockMessageReceiver());
    }
    return msg_receiver.load();
}

void MockReceiverResource::OnDataReceived(
        const octet* buf,
        const uint32_t,
        const Locator_t&,
        const Locator_t&)
{
    MockMessageReceiver* recv = msg_receiver.load();
    if (recv != nullptr)
    {
        recv->processCDRMsg(const_cast<octet*>(buf));
    }
}

void MockMessageReceiver::setCallback(
        std::function<void()> cb)
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    this->callback = cb;
}

void MockMessageReceiver::processCDRMsg(
        octet* buffer)
{
    data = buffer;
    std::lock_guard<std::mutex> lock(callback_mutex);
    if (callback != nullptr)
    {
        callback();
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
