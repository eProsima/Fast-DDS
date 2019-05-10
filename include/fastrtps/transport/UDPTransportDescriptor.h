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

#ifndef UDP_TRANSPORT_DESCRIPTOR
#define UDP_TRANSPORT_DESCRIPTOR

#include "./SocketTransportDescriptor.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TransportInterface;

/**
 * UDP Transport configuration
 *
 * - bufferSize:    length of the buffers used for transmission. Passing
 *                  a buffer of different size will cause transmission to
 *                  fail.
 *
 * - interfaceWhiteList: Lists the allowed interfaces.
 * @ingroup TRANSPORT_MODULE
 */
typedef struct UDPTransportDescriptor: public SocketTransportDescriptor
{
   virtual ~UDPTransportDescriptor(){}

   RTPS_DllAPI UDPTransportDescriptor();

   RTPS_DllAPI UDPTransportDescriptor(const UDPTransportDescriptor& t);

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
} UDPTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
