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

#ifndef PAYLOAD_COMPRESSION_TRANSPORT_H
#define PAYLOAD_COMPRESSION_TRANSPORT_H

#include "../ChainingTransport.h"
#include "PayloadCompressionTransportDescriptor.h"

#if HAVE_ZLIB || HAVE_BZIP2

#include <mutex>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct PayloadCompressionOptions;

/**
 * An adapter transport for bandwidth reduction.
 * This transport performs a standard compression of data before sending and the
 * corresponding decompression after receiving. The compression algorithm may be
 * selected between zlib and bzip2, or even let the transport perform both and
 * use the one which produces the shortest output.
 * This transport is configured using the following participant properties:
 * - rtps.payload_compression.compression_library: ZLIB, BZIP2 or AUTOMATIC
 * - rtps.payload_compression.compression_level: n (1 to 9)
 * - rtps.payload_compression.compression_library.small_packets: ZLIB, BZIP2 or AUTOMATIC
 * - rtps.payload_compression.compression_level.small_packets: n (1 to 9)
 * - rtps.payload_compression.compression_library.medium_packets: ZLIB, BZIP2 or AUTOMATIC
 * - rtps.payload_compression.compression_level.medium_packets: n (1 to 9)
 * - rtps.payload_compression.compression_library.large_packets: ZLIB, BZIP2 or AUTOMATIC
 * - rtps.payload_compression.compression_level.large_packets: n (1 to 9)
 * - rtps.payload_compression.low_mark: num_bytes
 * - rtps.payload_compression.high_mark: num_bytes
 * @ingroup TRANSPORT_MODULE
 */
class PayloadCompressionTransport : public ChainingTransport
{

public:

    RTPS_DllAPI PayloadCompressionTransport(
            const PayloadCompressionTransportDescriptor&);

    virtual ~PayloadCompressionTransport();

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
            const fastrtps::rtps::Locator_t& remote_locator,
            const std::chrono::microseconds& timeout) override;

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

    //! Size of the underlying transport buffer.
    uint32_t buffer_size_;
    //! Transport options. Filled when calling to init.
    std::unique_ptr<PayloadCompressionOptions> options_;
    //! Only one thread should have access to the compression buffer
    mutable std::mutex compress_buffer_mutex_;
    //! Compression buffer
    fastrtps::rtps::octet* compress_buffer_;
    //! Compression buffer size
    uint32_t compress_buffer_len_;
    //! Transport configuration
    PayloadCompressionTransportDescriptor configuration_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

#endif
