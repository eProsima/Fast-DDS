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

#include <rtps/network/ReceiverResource.h>

#include <cassert>
#include <thread>

#include <fastdds/dds/log/Log.hpp>

#include <rtps/messages/MessageReceiver.h>

#define IDSTRING "(ID:" << std::this_thread::get_id() << ") " <<

using namespace std;

namespace eprosima {
namespace fastdds {
namespace rtps {

ReceiverResource::ReceiverResource(
        TransportInterface& transport,
        const Locator_t& locator,
        uint32_t max_recv_buffer_size)
    : Cleanup(nullptr)
    , LocatorMapsToManagedChannel(nullptr)
    , mValid(false)
    , mtx()
    , cv_()
    , receiver(nullptr)
    , max_message_size_(max_recv_buffer_size)
    , active_callbacks_(0)
{
    // Internal channel is opened and assigned to this resource.
    mValid = transport.OpenInputChannel(locator, this, max_message_size_);
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
                return locator.kind == locatorToCheck.kind && transport.DoInputLocatorsMatch(locator, locatorToCheck);
            };
}

ReceiverResource::ReceiverResource(
        ReceiverResource&& rValueResource)
{
    std::lock_guard<std::mutex> _(mtx);

    Cleanup.swap(rValueResource.Cleanup);
    LocatorMapsToManagedChannel.swap(rValueResource.LocatorMapsToManagedChannel);
    receiver = rValueResource.receiver;
    rValueResource.receiver = nullptr;
    mValid = rValueResource.mValid;
    rValueResource.mValid = false;
    max_message_size_ = rValueResource.max_message_size_;
    active_callbacks_ = rValueResource.active_callbacks_;
    rValueResource.active_callbacks_ = 0;
}

bool ReceiverResource::SupportsLocator(
        const Locator_t& localLocator)
{
    if (LocatorMapsToManagedChannel)
    {
        return LocatorMapsToManagedChannel(localLocator);
    }
    return false;
}

void ReceiverResource::RegisterReceiver(
        MessageReceiver* rcv)
{
    std::lock_guard<std::mutex> _(mtx);

    if (receiver == nullptr)
    {
        receiver = rcv;
    }
}

void ReceiverResource::UnregisterReceiver(
        MessageReceiver* rcv)
{
    std::lock_guard<std::mutex> _(mtx);

    if (receiver == rcv)
    {
        receiver = nullptr;
    }
}

void ReceiverResource::OnDataReceived(
        const octet* data,
        const uint32_t size,
        const Locator_t& localLocator,
        const Locator_t& remoteLocator)
{
    (void)localLocator;

    std::lock_guard<std::mutex> _(mtx);

    MessageReceiver* rcv = receiver;

    if (rcv != nullptr && active_callbacks_ >= 0)
    {
        ++active_callbacks_;

        CDRMessage_t msg(0);
        msg.wraps = true;
        msg.buffer = const_cast<octet*>(data);
        msg.length = size;
        msg.max_size = size;
        msg.reserved_size = size;

        // TODO: Should we unlock in case UnregisterReceiver is called from callback ?
        rcv->processCDRMsg(remoteLocator, localLocator, &msg);

        // allow disabling
        if (--active_callbacks_ == 0)
        {
            cv_.notify_one();
        }
    }
}

void ReceiverResource::disable()
{
    if (Cleanup)
    {
        Cleanup();
    }

    // wait until all callbacks are finished
    std::unique_lock<std::mutex> lock(mtx);
    cv_.wait(lock, [this]
            {
                return active_callbacks_ <= 0;
            });
    // no more callbacks
    active_callbacks_ = -1;
}

ReceiverResource::~ReceiverResource()
{
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
