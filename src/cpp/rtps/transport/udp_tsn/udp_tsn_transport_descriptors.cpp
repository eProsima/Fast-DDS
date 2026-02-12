/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/udp_tsn/TSN_UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/udp_tsn/TSN_UDPv6TransportDescriptor.hpp>

#include <rtps/transport/udp_tsn/TSN_UDPv4Transport.hpp>
#include <rtps/transport/udp_tsn/TSN_UDPv6Transport.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

TSN_UDPv4TransportDescriptor::TSN_UDPv4TransportDescriptor()
    : UDPv4TransportDescriptor()
{
}

bool TSN_UDPv4TransportDescriptor::operator ==(
        const TSN_UDPv4TransportDescriptor& t) const
{
    return (this->priority_mapping == t.priority_mapping) &&
           (UDPv4TransportDescriptor::operator ==(t));
}

TransportInterface* TSN_UDPv4TransportDescriptor::create_transport() const
{
    return new TSN_UDPv4Transport(*this);
}

TSN_UDPv6TransportDescriptor::TSN_UDPv6TransportDescriptor()
    : UDPv6TransportDescriptor()
{
}

bool TSN_UDPv6TransportDescriptor::operator ==(
        const TSN_UDPv6TransportDescriptor& t) const
{
    return (this->priority_mapping == t.priority_mapping) &&
           (UDPv6TransportDescriptor::operator ==(t));
}

TransportInterface* TSN_UDPv6TransportDescriptor::create_transport() const
{
    return new TSN_UDPv6Transport(*this);
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
