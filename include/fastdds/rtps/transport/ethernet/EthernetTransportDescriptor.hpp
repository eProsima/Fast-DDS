/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_RTPS_TRANSPORT_ETHERNET__ETHERNETTRANSPORTDESCRIPTOR_HPP_
#define FASTDDS_RTPS_TRANSPORT_ETHERNET__ETHERNETTRANSPORTDESCRIPTOR_HPP_

#include <cstdint>
#include <map>
#include <string>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Ethernet Transport configuration and factory.
 */
struct EthernetTransportDescriptor : public PortBasedTransportDescriptor
{
    /// Mapping of source port, PCP and VLAN ID for a given priority.
    struct PriorityMapping
    {
        uint16_t source_port = 0;
        uint8_t pcp = 0;
        uint16_t vlan_id = 0;
    };

    FASTDDS_EXPORTED_API EthernetTransportDescriptor();

    FASTDDS_EXPORTED_API TransportInterface* create_transport() const override;

    FASTDDS_EXPORTED_API uint32_t min_send_buffer_size() const override
    {
        return send_buffer_size;
    }

    //! Length of the send buffer.
    uint32_t send_buffer_size = 0;
    //! Length of the receive buffer.
    uint32_t receive_buffer_size = 0;
    //! Name of the network interface to be used (e.g. "eth0").
    std::string interface_name;
    //! Default value for the source port field in the packet prefix.
    uint16_t default_source_port = 0;
    //! Mapping of transport priority to source port, PCP and VLAN ID.
    std::map<int32_t, PriorityMapping> priority_mapping;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_RTPS_TRANSPORT_ETHERNET__ETHERNETTRANSPORTDESCRIPTOR_HPP_
