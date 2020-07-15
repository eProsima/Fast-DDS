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

#ifndef MOCK_RECEIVER_STUFF_H
#define MOCK_RECEIVER_STUFF_H

#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <fastrtps/transport/ChannelResource.h>
#include <functional>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class MockMessageReceiver;

class MockReceiverResource : public ReceiverResource
{
public:
    virtual void OnDataReceived(const octet*, const uint32_t,
        const Locator_t&, const Locator_t&) override;
    MockReceiverResource(TransportInterface& transport, const Locator_t& locator);
    ~MockReceiverResource();
    MessageReceiver* CreateMessageReceiver() override;
    MockMessageReceiver* msg_receiver;
};

class MockMessageReceiver : public MessageReceiver
{
public:
    MockMessageReceiver() : MessageReceiver(nullptr, nullptr) {}
    void processCDRMsg(const Locator_t& loc, CDRMessage_t*msg) override;
    void setCallback(std::function<void()> cb);
    octet* data;
    std::function<void()> callback;
    uint32_t data_size;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif //MOCK_RECEIVER_STUFF_H