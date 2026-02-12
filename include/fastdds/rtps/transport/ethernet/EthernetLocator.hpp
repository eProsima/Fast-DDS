/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef FASTDDS_RTPS_TRANSPORT_ETHERNET__ETHERNETLOCATOR_HPP_
#define FASTDDS_RTPS_TRANSPORT_ETHERNET__ETHERNETLOCATOR_HPP_

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Locator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/// Size of an Ethernet address in bytes.
constexpr size_t ETHERNET_ADDRESS_SIZE = 6;

/// An ethernet address.
using EthernetAddress = std::array<uint8_t, ETHERNET_ADDRESS_SIZE>;

/// Ethernet locator, used to perform easy conversion between Ethernet addresses and RTPS locators.
struct FASTDDS_EXPORTED_API EthernetLocator
{
    /// Specifies the locator type.
    int32_t kind;
    /// Network port
    uint32_t port;
    /// Padding
    std::array<uint8_t, 10> padding;
    /// Ethernet address
    EthernetAddress address;

    /**
     * @brief Get the PCP, DEI, and VLAN ID from the port field.
     *
     * @return A 16-bit unsigned integer where the upper 3 bits are PCP,
     *         the next bit is DEI (always 0), and the lower 12 bits are VLAN ID.
     */
    inline uint16_t get_pcp_dei_vid() const
    {
        uint16_t pcp = static_cast<uint16_t>((port >> 16) & 0x0FU);
        uint16_t vlan_id = static_cast<uint16_t>((port >> 20) & 0x0FFFU);
        return (pcp << 13) | vlan_id;
    }

    /**
     * @brief Get the logical port from the locator.
     *
     * @param locator The locator from which to extract the logical port.
     *
     * @return The logical port as a 16-bit unsigned integer.
     */
    static inline uint16_t get_logical_port(
            const Locator& locator)
    {
        return static_cast<uint16_t>(locator.port & 0xFFFF);
    }

    /**
     * @brief Set the port field from a logical port, VLAN ID, and PCP.
     *
     * @param logical_port The logical port number (0-65535).
     * @param vlan_id The VLAN ID (0-4094).
     * @param pcp The PCP value (0-7).
     */
    inline void set_port_vlan_pcp(
            uint16_t logical_port,
            uint16_t vlan_id,
            uint8_t pcp)
    {
        assert(vlan_id <= 4094);
        assert(pcp <= 7);
        port = static_cast<uint32_t>(logical_port) |
                (static_cast<uint32_t>(pcp & 0x07U) << 16) |
                (static_cast<uint32_t>(vlan_id & 0x0FFFU) << 20);
    }

    /**
     * @brief Create an Ethernet locator from an Ethernet address and port details.
     *
     * @param addr          The Ethernet address.
     * @param logical_port  The logical port number.
     * @param vlan_id       The VLAN ID (0-4094).
     * @param pcp           The PCP value (0-7).
     *
     * @return A Locator with kind LOCATOR_KIND_ETHERNET, the given Ethernet address,
     *         and a port field encoding the logical port, VLAN ID, and PCP.
     */
    static inline Locator create_locator(
            const EthernetAddress& addr,
            uint16_t logical_port,
            uint16_t vlan_id,
            uint8_t pcp)
    {
        Locator locator;
        locator.kind = LOCATOR_KIND_ETHERNET;
        EthernetLocator& eth_locator = reinterpret_cast<EthernetLocator&>(locator);
        eth_locator.set_port_vlan_pcp(logical_port, vlan_id, pcp);
        eth_locator.address = addr;
        eth_locator.padding.fill(0);
        eth_locator.padding[0] = 0xFF;
        return locator;
    }

    /**
     * @brief Create an Ethernet locator from an Ethernet address and port details.
     *
     * @param addr          The Ethernet address.
     * @param logical_port  The logical port number.
     * @param vlan_id       The VLAN ID (0-4094).
     * @param pcp           The PCP value (0-7).
     *
     * @return A Locator with kind LOCATOR_KIND_ETHERNET, the given Ethernet address,
     *         and a port field encoding the logical port, VLAN ID, and PCP.
     */
    static inline Locator create_locator(
            const uint8_t addr[ETHERNET_ADDRESS_SIZE],
            uint16_t logical_port,
            uint16_t vlan_id,
            uint8_t pcp)
    {
        Locator locator;
        locator.kind = LOCATOR_KIND_ETHERNET;
        EthernetLocator& eth_locator = reinterpret_cast<EthernetLocator&>(locator);
        eth_locator.set_port_vlan_pcp(logical_port, vlan_id, pcp);
        memcpy(eth_locator.address.data(), addr, eth_locator.address.size());
        eth_locator.padding.fill(0);
        eth_locator.padding[0] = 0xFF;
        return locator;
    }

    /**
     * @brief Check if the given locator is Ethernet and multicast.
     *
     * @param locator The locator to check.
     *
     * @return True if the locator is Ethernet and has a multicast address, false otherwise.
     */
    static inline bool is_ethernet_multicast(
            const Locator& locator)
    {
        if (locator.kind != LOCATOR_KIND_ETHERNET)
        {
            return false;
        }

        // Check if the first byte of the Ethernet address is multicast
        const EthernetAddress& addr = reinterpret_cast<const EthernetLocator&>(locator).address;
        return (addr[0] & 0x01) != 0;
    }

    /**
     * @brief Check if the given address is multicast.
     *
     * @param address The address to check.
     *
     * @return True if the address is a multicast address, false otherwise.
     */
    static inline bool is_ethernet_multicast(
            const EthernetAddress& address)
    {
        return (address[0] & 0x01) != 0;
    }

    /**
     * @brief Check if the given locator is Ethernet and has an all-zero address.
     *
     * @param locator The locator to check.
     *
     * @return True if the locator is Ethernet and has an all-zero address, false otherwise.
     */
    static inline bool is_ethernet_any(
            const Locator& locator)
    {
        if (locator.kind != LOCATOR_KIND_ETHERNET)
        {
            return false;
        }

        // Check if the address is all zeros
        return !IsAddressDefined(locator);
    }

};

static_assert(
    sizeof(EthernetLocator) == sizeof(Locator),
    "EthernetLocator must have the same size as Locator");

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_RTPS_TRANSPORT_ETHERNET__ETHERNETLOCATOR_HPP_
