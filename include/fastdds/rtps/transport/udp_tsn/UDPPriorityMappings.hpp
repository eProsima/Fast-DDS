/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */
/**
 * @file UDPPriorityMappings.hpp
 *
 */

#ifndef FASTDDS_RTPS_TRANSPORT_UDP_TSN__UDPPRIORITYMAPPINGS_HPP
#define FASTDDS_RTPS_TRANSPORT_UDP_TSN__UDPPRIORITYMAPPINGS_HPP

#include <cstdint>
#include <string>
#include <map>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief UDP Priority Mapping configuration entry.
 *
 * @ingroup TRANSPORT_MODULE
 */
struct UDPPriorityMapping
{
    /// Differentiated Services Code Point (DSCP) value.
    uint8_t dscp = 0;
    /// Specific source port for this priority mapping. If 0, the default source port will be used.
    uint16_t source_port = 0;
    /// Name (e.g. "eth0") or IP address of the network interface to use. If empty, no restriction will be applied.
    std::string interface;

    /**
     * @brief Equality operator.
     *
     * @param other Other mapping to compare.
     *
     * @return True if both mappings are equal, false otherwise.
     */
    bool operator ==(
            const UDPPriorityMapping& other) const
    {
        return dscp == other.dscp && source_port == other.source_port && interface == other.interface;
    }

};

/**
 * @brief Map of transport priority to UDP Priority Mapping.
 */
using UDPPriorityMappings = std::map<int32_t, UDPPriorityMapping>;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_UDP_TSN__UDPPRIORITYMAPPINGS_HPP
