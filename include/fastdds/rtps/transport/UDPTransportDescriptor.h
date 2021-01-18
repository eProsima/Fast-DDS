// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_UDP_TRANSPORT_DESCRIPTOR_
#define _FASTDDS_UDP_TRANSPORT_DESCRIPTOR_

#include <fastdds/rtps/transport/SocketTransportDescriptor.h>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * UDP Transport configuration
 *
 * - m_output_udp_socket: source port to use for outgoing datagrams.
 *
 * - non_blocking_send: do not block on send operations. When it is set to true, send operations will return
 * immediately if the buffer is full, but no error will be returned to the upper layer. This means that the
 * application will behave as if the datagram is sent and lost.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct UDPTransportDescriptor : public SocketTransportDescriptor
{
    //! Destructor
    virtual ~UDPTransportDescriptor() = default;

    //! Constructor
    RTPS_DllAPI UDPTransportDescriptor();

    //! Copy constructor
    RTPS_DllAPI UDPTransportDescriptor(
            const UDPTransportDescriptor& t) = default;

    //! Copy assignment
    RTPS_DllAPI UDPTransportDescriptor& operator =(
            const UDPTransportDescriptor& t) = default;

    //! Comparison operator
    RTPS_DllAPI bool operator ==(
            const UDPTransportDescriptor& t) const;

    //! Source port to use for outgoing datagrams
    uint16_t m_output_udp_socket;

    /**
     * Whether to use non-blocking calls to send_to().
     *
     * When set to true, calls to send_to() will return inmediately if the buffer is full, but
     * no error will be returned to the upper layer. This means that the application will behave
     * as if the datagram is sent but lost (i.e. throughput may be reduced). This value is
     * specially useful on high-frequency best-effort writers.
     *
     * When set to false, calls to send_to() will block until the network buffer has space for the
     * datagram. This may hinder performance on high-frequency writers.
     */
    bool non_blocking_send = false;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_UDP_TRANSPORT_DESCRIPTOR_
