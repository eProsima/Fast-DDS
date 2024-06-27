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
 * @file TCPv4TransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__TCPV4TRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT__TCPV4TRANSPORTDESCRIPTOR_HPP

#include <sstream>

#include <fastdds/rtps/transport/TCPTransportDescriptor.hpp>
#include <fastdds/rtps/common/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * TCPv4 Transport configuration.
 * The kind value for TCPv4TransportDescriptor is given by \c eprosima::fastdds::rtps::LOCATOR_KIND_TCPv4.
 *
 * - \c wan_addr: Public IP address. Peers on a different LAN will use this IP for communications with this host.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct TCPv4TransportDescriptor : public TCPTransportDescriptor
{
    //! Destructor
    virtual ~TCPv4TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Public IP address
    fastdds::rtps::octet wan_addr[4];

    //! Set the public IP address
    void set_WAN_address(
            fastdds::rtps::octet o1,
            fastdds::rtps::octet o2,
            fastdds::rtps::octet o3,
            fastdds::rtps::octet o4)
    {
        wan_addr[0] = o1;
        wan_addr[1] = o2;
        wan_addr[2] = o3;
        wan_addr[3] = o4;
    }

    //! Set the public IP address
    void set_WAN_address(
            const std::string& in_address)
    {
        std::stringstream ss(in_address);
        int a, b, c, d; //to store the 4 ints
        char ch; //to temporarily store the '.'
        ss >> a >> ch >> b >> ch >> c >> ch >> d;
        wan_addr[0] = (fastdds::rtps::octet)a;
        wan_addr[1] = (fastdds::rtps::octet)b;
        wan_addr[2] = (fastdds::rtps::octet)c;
        wan_addr[3] = (fastdds::rtps::octet)d;
    }

    //! Get the public IP address
    std::string get_WAN_address()
    {
        std::stringstream ss;
        ss << static_cast<int>(wan_addr[0]) << "."
           << static_cast<int>(wan_addr[1]) << "."
           << static_cast<int>(wan_addr[2]) << "."
           << static_cast<int>(wan_addr[3]);
        return ss.str();
    }

    //! Constructor
    FASTDDS_EXPORTED_API TCPv4TransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API TCPv4TransportDescriptor(
            const TCPv4TransportDescriptor& t);

    //! Copy assignment
    FASTDDS_EXPORTED_API TCPv4TransportDescriptor& operator =(
            const TCPv4TransportDescriptor& t);

    //! Comparison operator
    FASTDDS_EXPORTED_API bool operator ==(
            const TCPv4TransportDescriptor& t) const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__TCPV4TRANSPORTDESCRIPTOR_HPP
