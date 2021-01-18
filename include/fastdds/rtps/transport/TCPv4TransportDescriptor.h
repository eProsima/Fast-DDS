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

#ifndef _FASTDDS_TCPV4_TRANSPORT_DESCRIPTOR_
#define _FASTDDS_TCPV4_TRANSPORT_DESCRIPTOR_

#include <sstream>

#include <fastdds/rtps/transport/TCPTransportDescriptor.h>
#include <fastdds/rtps/common/Types.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * TCPv4 Transport configuration.
 * The kind value for TCPv4TransportDescriptor is given by eprosima::fastrtps::rtps::LOCATOR_KIND_TCPv4.
 *
 * - wan_addr: Public IP address. Peers on a different LAN will use this IP for communications with this host.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct TCPv4TransportDescriptor : public TCPTransportDescriptor
{
    //! Destructor
    virtual ~TCPv4TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Public IP address
    fastrtps::rtps::octet wan_addr[4];

    //! Set the public IP address
    void set_WAN_address(
            fastrtps::rtps::octet o1,
            fastrtps::rtps::octet o2,
            fastrtps::rtps::octet o3,
            fastrtps::rtps::octet o4)
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
        wan_addr[0] = (fastrtps::rtps::octet)a;
        wan_addr[1] = (fastrtps::rtps::octet)b;
        wan_addr[2] = (fastrtps::rtps::octet)c;
        wan_addr[3] = (fastrtps::rtps::octet)d;
    }

    //! Constructor
    RTPS_DllAPI TCPv4TransportDescriptor();

    //! Copy constructor
    RTPS_DllAPI TCPv4TransportDescriptor(
            const TCPv4TransportDescriptor& t);

    //! Copy assignment
    RTPS_DllAPI TCPv4TransportDescriptor& operator =(
            const TCPv4TransportDescriptor& t);

    //! Comparison operator
    RTPS_DllAPI bool operator ==(
            const TCPv4TransportDescriptor& t) const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TCPV4_TRANSPORT_DESCRIPTOR_
