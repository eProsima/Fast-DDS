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
 * @file TCPv4TransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT__TCPV4TRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT__TCPV4TRANSPORTDESCRIPTOR_HPP

#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/transport/TCPTransportDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class TCPTransportInterface;
/**
 * Transport configuration
 * @ingroup TRANSPORT_MODULE
 */
typedef struct TCPv4TransportDescriptor : public TCPTransportDescriptor
{
    virtual ~TCPv4TransportDescriptor()
    {
    }

    virtual TransportInterface* create_transport() const override
    {
        return nullptr;
    }

    eprosima::fastdds::rtps::octet wan_addr[4];

    void set_WAN_address(
            eprosima::fastdds::rtps::octet o1,
            eprosima::fastdds::rtps::octet o2,
            eprosima::fastdds::rtps::octet o3,
            eprosima::fastdds::rtps::octet o4)
    {
        wan_addr[0] = o1;
        wan_addr[1] = o2;
        wan_addr[2] = o3;
        wan_addr[3] = o4;
    }

    void set_WAN_address(
            const std::string& in_address)
    {
        std::stringstream ss(in_address);
        int a, b, c, d; //to store the 4 ints
        char ch; //to temporarily store the '.'
        ss >> a >> ch >> b >> ch >> c >> ch >> d;
        wan_addr[0] = (eprosima::fastdds::rtps::octet)a;
        wan_addr[1] = (eprosima::fastdds::rtps::octet)b;
        wan_addr[2] = (eprosima::fastdds::rtps::octet)c;
        wan_addr[3] = (eprosima::fastdds::rtps::octet)d;
    }

    FASTDDS_EXPORTED_API TCPv4TransportDescriptor()
    {

    }

    FASTDDS_EXPORTED_API TCPv4TransportDescriptor(
            const TCPv4TransportDescriptor& /*t*/)
        : TCPv4TransportDescriptor()
    {

    }

} TCPv4TransportDescriptor;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__TCPV4TRANSPORTDESCRIPTOR_HPP
