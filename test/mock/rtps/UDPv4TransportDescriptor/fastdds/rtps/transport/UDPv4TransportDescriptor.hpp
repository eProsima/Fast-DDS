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

/**
 * @file UDPv4TransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__UDPV4TRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT__UDPV4TRANSPORTDESCRIPTOR_HPP

#include <fastdds/rtps/transport/UDPTransportDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using TransportInterface = fastdds::rtps::TransportInterface;

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
typedef struct UDPv4TransportDescriptor : public UDPTransportDescriptor
{
    virtual ~UDPv4TransportDescriptor()
    {
    }

    FASTDDS_EXPORTED_API UDPv4TransportDescriptor()
        : UDPTransportDescriptor()
    {

    }

    FASTDDS_EXPORTED_API UDPv4TransportDescriptor(
            const UDPv4TransportDescriptor& /*t*/)
        : UDPTransportDescriptor()
    {

    }

} UDPv4TransportDescriptor;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__UDPV4TRANSPORTDESCRIPTOR_HPP
