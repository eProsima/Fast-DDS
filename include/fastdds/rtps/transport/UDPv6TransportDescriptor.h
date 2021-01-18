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

#ifndef _FASTDDS_UDPV6_TRANSPORT_DESCRIPTOR_
#define _FASTDDS_UDPV6_TRANSPORT_DESCRIPTOR_

#include <fastdds/rtps/transport/UDPTransportDescriptor.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class TransportInterface;

/**
 * UDPv6 Transport configuration
 * The kind value for UDPv6TransportDescriptor is given by eprosima::fastrtps::rtps::LOCATOR_KIND_UDPv6.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct UDPv6TransportDescriptor : public UDPTransportDescriptor
{
    //! Destructor
    virtual ~UDPv6TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Constructor
    RTPS_DllAPI UDPv6TransportDescriptor();

    //! Copy constructor
    RTPS_DllAPI UDPv6TransportDescriptor(
            const UDPv6TransportDescriptor& t) = default;

    //! Copy assignment
    RTPS_DllAPI UDPv6TransportDescriptor& operator =(
            const UDPv6TransportDescriptor& t) = default;

    RTPS_DllAPI bool operator ==(
            const UDPv6TransportDescriptor& t) const;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_UDPV6_TRANSPORT_DESCRIPTOR_
