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

#ifndef UDPV4_TRANSPORT_DESCRIPTOR
#define UDPV4_TRANSPORT_DESCRIPTOR

#include "./UDPTransportDescriptor.h"

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
typedef struct UDPv4TransportDescriptor: public UDPTransportDescriptor
{
   virtual ~UDPv4TransportDescriptor(){}

   virtual TransportInterface* create_transport() const override;

   RTPS_DllAPI UDPv4TransportDescriptor();

   RTPS_DllAPI UDPv4TransportDescriptor(const UDPv4TransportDescriptor& t);

} UDPv4TransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
