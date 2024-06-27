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

/**
 * @file UDPTransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__UDPTRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT__UDPTRANSPORTDESCRIPTOR_HPP

#include <fastdds/rtps/transport/SocketTransportDescriptor.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * UDP Transport configuration
 *
 * - \c m_output_udp_socket: source port to use for outgoing datagrams.
 *
 * - \c non_blocking_send: do not block on send operations. When it is set to true, send operations will return
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
    FASTDDS_EXPORTED_API UDPTransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API UDPTransportDescriptor(
            const UDPTransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API UDPTransportDescriptor& operator =(
            const UDPTransportDescriptor& t) = default;

    //! Comparison operator
    FASTDDS_EXPORTED_API bool operator ==(
            const UDPTransportDescriptor& t) const;

    //! Source port to use for outgoing datagrams
    uint16_t m_output_udp_socket;

    /**
     * Whether to use non-blocking calls to send_to().
     *
     * When set to true, calls to send_to() will return immediately if the buffer is full, but
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

#endif // FASTDDS_RTPS_TRANSPORT__UDPTRANSPORTDESCRIPTOR_HPP
