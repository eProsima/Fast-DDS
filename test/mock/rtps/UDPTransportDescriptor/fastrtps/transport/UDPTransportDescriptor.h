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

#include <fastrtps/transport/SocketTransportDescriptor.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TransportInterface;

/**
 * Transport configuration
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

   virtual TransportInterface* create_transport() const override {	return nullptr;	}

   RTPS_DllAPI UDPTransportDescriptor()
   : SocketTransportDescriptor(65550, 4)
   {

   }

   RTPS_DllAPI UDPTransportDescriptor(const UDPTransportDescriptor& /*t*/)
   : SocketTransportDescriptor(65550, 4)
   {

   }

   uint16_t m_output_udp_socket;
   
   bool non_blocking_send = false;
} UDPTransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
