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

#ifndef TCPV6_TRANSPORT_DESCRIPTOR
#define TCPV6_TRANSPORT_DESCRIPTOR

#include <fastrtps/transport/TCPTransportDescriptor.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TCPTransportInterface;

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
typedef struct TCPv6TransportDescriptor: public TCPTransportDescriptor
{
    virtual ~TCPv6TransportDescriptor(){}

    virtual TransportInterface* create_transport() const override {	return nullptr;	}

    RTPS_DllAPI TCPv6TransportDescriptor()
    {

    }

    RTPS_DllAPI TCPv6TransportDescriptor(const TCPv6TransportDescriptor& /*t*/) : TCPv6TransportDescriptor()
    {

    }

} TCPv6TransportDescriptor;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
