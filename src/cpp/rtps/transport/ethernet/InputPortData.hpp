/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_ETH_TRANSPORT__SRC__INPUTPORTDATA_HPP_
#define FASTDDS_ETH_TRANSPORT__SRC__INPUTPORTDATA_HPP_

#include <cstdint>
#include <memory>

#include <rtps/transport/ethernet/EthernetPacket.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Class to process incoming Ethernet packets on a specific port.
 */
struct InputPortData
{
    std::shared_ptr<EthernetPacket> packet;  ///< Shared pointer to the received Ethernet packet
    EthernetPacketData* data;                ///< Pointer to the Ethernet packet data structure
    uint32_t payload_size;                   ///< Size of the RTPS payload within the packet data
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_ETH_TRANSPORT__SRC__INPUTPORTDATA_HPP_
