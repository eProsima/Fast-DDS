/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_ETH_TRANSPORT__SRC__ETHERNETPACKET_HPP_
#define FASTDDS_ETH_TRANSPORT__SRC__ETHERNETPACKET_HPP_

#include <cstdint>
#include <cstddef>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Ethernet header fields.
 *
 * This should match struct ethhdr in <linux/if_ether.h>.
 * It is defined here to avoid dependency on Linux headers, and make the code portable.
 */
struct EthernetPacketHeader
{
    /// Ethernet address length in bytes
    static constexpr size_t ETH_ADDR_LEN = 6;

    uint8_t h_dest[ETH_ADDR_LEN];     ///< Destination MAC address
    uint8_t h_source[ETH_ADDR_LEN];   ///< Source MAC address
    uint16_t h_proto;                 ///< Ethertype / VLAN tag
};

/**
 * @brief Ethernet packet prefix fields.
 *
 * This structure contains the fields that precede the RTPS payload in an Ethernet packet.
 * It includes VLAN tagging and RTPS logical port numbers.
 */
struct EthernetPacketPrefix
{
    static constexpr size_t ETH_MIN_SIZE = 46;      ///< Minimum payload size for Ethernet frames
    static constexpr size_t ETH_MTU = 1500;         ///< Maximum Transmission Unit for Ethernet
    static constexpr uint16_t ETH_P_RTPS = 0xEDD5;  ///< Ethertype for RTPS

    uint16_t pcp_dei_vid;  ///< Priority Code Point (PCP), Drop Eligible Indicator (DEI), and VLAN ID
    uint16_t proto;        ///< Ethertype field
    uint16_t source_port;  ///< RTPS source logical port number
    uint16_t dest_port;    ///< RTPS destination logical port number
};

/**
 * @brief Full RTPS ethernet packet.
 *
 * This structure represents a complete Ethernet packet used in RTPS communication.
 */
struct EthernetPacket
{
    /// Maximum size of the RTPS payload in an Ethernet packet.
    static constexpr size_t MAX_RTPS_PAYLOAD_SIZE = EthernetPacketPrefix::ETH_MTU - sizeof(EthernetPacketPrefix);
    static constexpr size_t MIN_RTPS_PAYLOAD_SIZE = EthernetPacketPrefix::ETH_MIN_SIZE - sizeof(EthernetPacketPrefix);

    EthernetPacketHeader header;             ///< Ethernet packet header
    EthernetPacketPrefix prefix;             ///< Ethernet packet prefix
    uint8_t payload[MAX_RTPS_PAYLOAD_SIZE];  ///< Payload of the Ethernet packet
};

/**
 * @brief Structure representing the data contained in an Ethernet packet.
 *
 * This structure is used to access the RTPS logical port numbers and payload
 * within an Ethernet packet.
 */
struct EthernetPacketData
{
    uint16_t source_port;                                    ///< RTPS source logical port number
    uint16_t dest_port;                                      ///< RTPS destination logical port number
    uint8_t payload[EthernetPacket::MAX_RTPS_PAYLOAD_SIZE];  ///< Payload of the Ethernet packet
};

// Ensure that the size of EthernetPacket matches the IEEE 802.3 standard for Ethernet frames.
static_assert(sizeof(EthernetPacket) == 1514, "Unexpected size of EthernetPacket");

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_ETH_TRANSPORT__SRC__ETHERNETPACKET_HPP_
