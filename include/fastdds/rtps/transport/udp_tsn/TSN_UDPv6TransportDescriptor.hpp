/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */
/**
 * @file TSN_UDPv6TransportDescriptor.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT_UDP_TSN__TSN_UDPV6TRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT_UDP_TSN__TSN_UDPV6TRANSPORTDESCRIPTOR_HPP

#include <fastdds/fastdds_dll.hpp>

#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/udp_tsn/UDPPriorityMappings.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * TSN aware UDPv6 Transport configuration.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct TSN_UDPv6TransportDescriptor : public UDPv6TransportDescriptor
{
    //! Destructor
    virtual ~TSN_UDPv6TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Constructor
    FASTDDS_EXPORTED_API TSN_UDPv6TransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API TSN_UDPv6TransportDescriptor(
            const TSN_UDPv6TransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API TSN_UDPv6TransportDescriptor& operator =(
            const TSN_UDPv6TransportDescriptor& t) = default;

    FASTDDS_EXPORTED_API bool operator ==(
            const TSN_UDPv6TransportDescriptor& t) const;

    //! Mapping of transport priority to DSCP, source port and interface.
    UDPPriorityMappings priority_mapping;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_UDP_TSN__TSN_UDPV6TRANSPORTDESCRIPTOR_HPP
