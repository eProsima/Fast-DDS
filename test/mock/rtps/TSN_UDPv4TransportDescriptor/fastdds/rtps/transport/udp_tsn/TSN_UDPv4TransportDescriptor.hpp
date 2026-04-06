/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */
/**
 * @file TSN_UDPv4TransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT_UDP_TSN__TSN_UDPV4TRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT_UDP_TSN__TSN_UDPV4TRANSPORTDESCRIPTOR_HPP

#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/udp_tsn/UDPPriorityMappings.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct TSN_UDPv4TransportDescriptor : public UDPv4TransportDescriptor
{
    virtual ~TSN_UDPv4TransportDescriptor() = default;

    FASTDDS_EXPORTED_API TSN_UDPv4TransportDescriptor()
        : UDPv4TransportDescriptor()
    {
    }

    FASTDDS_EXPORTED_API TSN_UDPv4TransportDescriptor(
            const TSN_UDPv4TransportDescriptor& t) = default;

    FASTDDS_EXPORTED_API TSN_UDPv4TransportDescriptor& operator =(
            const TSN_UDPv4TransportDescriptor& t) = default;

    virtual TransportInterface* create_transport() const override
    {
        return nullptr;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const TSN_UDPv4TransportDescriptor& t) const
    {
        return priority_mapping == t.priority_mapping;
    }

    UDPPriorityMappings priority_mapping;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_UDP_TSN__TSN_UDPV4TRANSPORTDESCRIPTOR_HPP
