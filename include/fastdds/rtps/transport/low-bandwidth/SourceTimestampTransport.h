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

#ifndef SOURCE_TIMESTAMP_TRANSPORT_H
#define SOURCE_TIMESTAMP_TRANSPORT_H

#include "../ChainingTransport.h"
#include "SourceTimestampTransportDescriptor.h"

#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This transport will prepend a short (5 bytes) header to every packet, transmitting the timestamp seconds of the sender.
 * It will extract that information upon reception, and then invoke a callback informing of the timestamp (in seconds) of
 * the sender and the current timestamp of the receiver.
 * This callback and its generic parameter should be configured on the transport descriptor.
 * @ingroup TRANSPORT_MODULE
 */
class SourceTimestampTransport : public ChainingTransport
{

public:

    RTPS_DllAPI SourceTimestampTransport(
            const SourceTimestampTransportDescriptor&);

    virtual ~SourceTimestampTransport();

    virtual bool init(
            const fastrtps::rtps::PropertyPolicy* properties = nullptr) override;

    TransportDescriptorInterface* get_configuration() override
    {
        return &configuration_;
    }

    /**
     * Blocking Send through the specified channel. It will prepend the timestamp information and then send
     * the data to the lower transport.
     * @param sendBuffer Slice into the raw data to send.
     * @param sendBufferSize Size of the raw data. It will be used as a bounds check for the previous argument.
     * It must not exceed the sendBufferSize fed to this class during construction.
     * @param localLocator Locator mapping to the channel we're sending from.
     * @param remoteLocator Locator describing the remote destination we're sending to.
     */
    bool send(
            fastrtps::rtps::SenderResource* low_sender_resource,
            const fastrtps::rtps::octet* send_buffer,
            uint32_t send_buffer_size,
            fastrtps::rtps::LocatorsIterator* destination_locators_begin,
            fastrtps::rtps::LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& timeout) override;

    /**
     * Blocking Receive from the specified channel. It will extract the timestamp information and perform
     * the callback invocation.
     * @param receiveBuffer vector with enough capacity (not size) to accomodate a full receive buffer. That
     * capacity must not be less than the receiveBufferSize supplied to this class during construction.
     * @param localLocator Locator mapping to the local channel we're listening to.
     * @param[out] remoteLocator Locator describing the remote restination we received a packet from.
     */
    void receive(
            TransportReceiverInterface* next_receiver,
            const fastrtps::rtps::octet* receive_buffer,
            uint32_t receive_buffer_size,
            const fastrtps::rtps::Locator_t& local_locator,
            const fastrtps::rtps::Locator_t& remote_locator) override;

protected:

    //! Transport configuration including callback and buffer size
    SourceTimestampTransportDescriptor configuration_;
    //! Only one thread should have access to the compression buffer
    mutable std::recursive_mutex compress_buffer_mutex_;
    //! Compression buffer
    fastrtps::rtps::octet* compress_buffer_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
