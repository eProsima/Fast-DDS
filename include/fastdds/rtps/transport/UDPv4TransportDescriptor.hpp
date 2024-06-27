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

/**
 * UDPv4 Transport configuration
 * The kind value for UDPv4TransportDescriptor is given by \c eprosima::fastdds::rtps::LOCATOR_KIND_UDPv4.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct UDPv4TransportDescriptor : public UDPTransportDescriptor
{
    //! Destructor
    virtual ~UDPv4TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Constructor
    FASTDDS_EXPORTED_API UDPv4TransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API UDPv4TransportDescriptor(
            const UDPv4TransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API UDPv4TransportDescriptor& operator =(
            const UDPv4TransportDescriptor& t) = default;

    FASTDDS_EXPORTED_API bool operator ==(
            const UDPv4TransportDescriptor& t) const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__UDPV4TRANSPORTDESCRIPTOR_HPP
