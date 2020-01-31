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
#include <fastdds/rtps/messages/MessageReceiver.h>
#include <fastdds/rtps/transport/TransportInterface.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * RAII object that encapsulates the Receive operation over one channel in an unknown transport.
 * A Receiver resource is always univocally associated to a transport channel; the
 * act of constructing a Receiver Resource opens the channel and its destruction
 * closes it.
 * @ingroup NETWORK_MODULE
 */
class ReceiverResource : public fastdds::rtps::TransportReceiverInterface
{
   //! Only NetworkFactory is ever allowed to construct a ReceiverResource from scratch.
   //! In doing so, it guarantees the transport and channel are in a valid state for
   //! this resource to exist.
    friend class NetworkFactory;

public:
    /**
    * Method called by the transport when receiving data.
    * @param data Pointer to the received data.
    * @param size Number of bytes received.
    * @param localLocator Locator identifying the local endpoint.
    * @param remoteLocator Locator identifying the remote endpoint.
    */
    virtual void OnDataReceived(const octet* data, const uint32_t size,
        const Locator_t& localLocator, const Locator_t& remoteLocator) override;

    /**
     * Reports whether this resource supports the given local locator (i.e., said locator
     * maps to the transport channel managed by this resource).
     */
    bool SupportsLocator(const Locator_t& localLocator);

    /**
     * Register a MessageReceiver object to be called upon reception of data.
     * @param receiver The message receiver to register.
     */
    void RegisterReceiver(MessageReceiver* receiver);

    /**
    * Unregister a MessageReceiver object to be called upon reception of data.
    * @param receiver The message receiver to unregister.
    */
    void UnregisterReceiver(MessageReceiver* receiver);

    /**
     * Closes related ChannelResources.
     */
    void disable();

    inline uint32_t max_message_size() const
    {
        return max_message_size_;
    }

    /**
     * Resources can only be transfered through move semantics. Copy, assignment, and
     * construction outside of the factory are forbidden.
     */
    ReceiverResource(ReceiverResource&&);

    ~ReceiverResource() override;

private:
    ReceiverResource() = delete;
    ReceiverResource(const ReceiverResource&) = delete;
    ReceiverResource& operator=(const ReceiverResource&) = delete;

    ReceiverResource(fastdds::rtps::TransportInterface&, const Locator_t&, uint32_t);
    std::function<void()> Cleanup;
    std::function<bool(const Locator_t&)> LocatorMapsToManagedChannel;
    bool mValid; // Post-construction validity check for the NetworkFactory

    std::mutex mtx;
    MessageReceiver* receiver;
    uint32_t max_message_size_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif /* _FASTDDS_RTPS_RECEIVER_RESOURCE_H */
