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
 * @file IPLocator.hpp
 *
 */

#ifndef FASTDDS_UTILS__IPLOCATOR_HPP
#define FASTDDS_UTILS__IPLOCATOR_HPP

#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <regex>
#include <set>
#include <string>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace rtps {

class Locator_t;

/**
 * Class IPLocator, to provide helper functions to the IP based transports.
 * @ingroup UTILITIES_MODULE
 */
class IPLocator
{
public:

    /**
     * Fills locator with the given parameters.
     * @param kindin Kind of the locator.
     * @param address IP Address of the locator as string.
     * @param portin Port of the locator.
     * @param locator Locator to be filled.
     */
    FASTDDS_EXPORTED_API static void createLocator(
            int32_t kindin,
            const std::string& address,
            uint32_t portin,
            Locator_t& locator);

    //! Sets locator's IPv4.
    FASTDDS_EXPORTED_API static bool setIPv4(
            Locator_t& locator,
            const unsigned char* addr);

    //! Sets locator's IPv4.
    FASTDDS_EXPORTED_API static bool setIPv4(
            Locator_t& locator,
            octet o1,
            octet o2,
            octet o3,
            octet o4);

    //! Sets locator's IPv4.
    FASTDDS_EXPORTED_API static bool setIPv4(
            Locator_t& locator,
            const std::string& ipv4);

    //! Copies locator's IPv4.
    FASTDDS_EXPORTED_API static bool setIPv4(
            Locator_t& destlocator,
            const Locator_t& origlocator);

    //! Copies locator's IPv4.
    FASTDDS_EXPORTED_API static bool setIPv4address(
            Locator_t& destlocator,
            const std::string& lan,
            const std::string& wan,
            const std::string& ipv4);

    //! Retrieves locator's IPv4 as octet array.
    FASTDDS_EXPORTED_API static const octet* getIPv4(
            const Locator_t& locator);

    //! Check if the locator has IPv4.
    FASTDDS_EXPORTED_API static bool hasIPv4(
            const Locator_t& locator);

    //! Returns a string representation of the locator's IPv4.
    FASTDDS_EXPORTED_API static std::string toIPv4string(
            const Locator_t& locator);

    /**
     * @brief Copies locator's IPv4 to a destination array.
     * @param locator Locator from which to copy the IPv4.
     * @param dest Destination array where the IPv4 will be copied.
     * @return true if the copy was successful, false otherwise.
     */
    FASTDDS_EXPORTED_API static bool copyIPv4(
            const Locator_t& locator,
            unsigned char* dest);

    /**
     * @brief Copies locator's IPv4 to a destination locator.
     *        It only copies the IPv4 part (last 4 bytes), leaving other parts unchanged.
     * @param locator Locator from which to copy the IPv4.
     * @param dest Destination locator where the IPv4 will be copied.
     * @return true if the copy was successful, false otherwise.
     */
    FASTDDS_EXPORTED_API static bool copyIPv4(
            const Locator_t& locator,
            Locator_t& dest);

    // IPv6
    //! Sets locator's IPv6.
    FASTDDS_EXPORTED_API static bool setIPv6(
            Locator_t& locator,
            const unsigned char* addr);

    //! Sets locator's IPv6.
    FASTDDS_EXPORTED_API static bool setIPv6(
            Locator_t& locator,
            uint16_t group0,
            uint16_t group1,
            uint16_t group2,
            uint16_t group3,
            uint16_t group4,
            uint16_t group5,
            uint16_t group6,
            uint16_t group7);

    //! Sets locator's IPv6.
    FASTDDS_EXPORTED_API static bool setIPv6(
            Locator_t& locator,
            const std::string& ipv6);

    //! Copies locator's IPv6.
    FASTDDS_EXPORTED_API static bool setIPv6(
            Locator_t& destlocator,
            const Locator_t& origlocator);

    //! Retrieves locator's IPv6 as octet array.
    FASTDDS_EXPORTED_API static const octet* getIPv6(
            const Locator_t& locator);

    //! Check if the locator has IPv6.
    FASTDDS_EXPORTED_API static bool hasIPv6(
            const Locator_t& locator);

    //! Returns a string representation of the locator's IPv6 following RFC 5952 recommendation.
    FASTDDS_EXPORTED_API static std::string toIPv6string(
            const Locator_t& locator);

    //! Copies locator's IPv6.
    FASTDDS_EXPORTED_API static bool copyIPv6(
            const Locator_t& locator,
            unsigned char* dest);

    //! Sets locator's IP
    FASTDDS_EXPORTED_API static bool ip(
            Locator_t& locator,
            const std::string& ip);

    //! Returns a string representation of the locator's IP.
    FASTDDS_EXPORTED_API static std::string ip_to_string(
            const Locator_t& locator);

    // TCP
    //! Sets locator's logical port (as in RTCP protocol)
    FASTDDS_EXPORTED_API static bool setLogicalPort(
            Locator_t& locator,
            uint16_t port);

    //! Gets locator's logical port (as in RTCP protocol)
    FASTDDS_EXPORTED_API static uint16_t getLogicalPort(
            const Locator_t& locator);

    //! Sets locator's physical port (as in RTCP protocol)
    FASTDDS_EXPORTED_API static bool setPhysicalPort(
            Locator_t& locator,
            uint16_t port);

    //! Gets locator's physical port (as in RTCP protocol)
    FASTDDS_EXPORTED_API static uint16_t getPhysicalPort(
            const Locator_t& locator);

    // TCPv4
    //! Sets locator's WAN address (as in RTCP protocol)
    FASTDDS_EXPORTED_API static bool setWan(
            Locator_t& locator,
            octet o1,
            octet o2,
            octet o3,
            octet o4);

    //! Sets locator's WAN address (as in RTCP protocol)
    FASTDDS_EXPORTED_API static bool setWan(
            Locator_t& locator,
            const std::string& wan);

    //! Gets locator's WAN address (as in RTCP protocol)
    FASTDDS_EXPORTED_API static const octet* getWan(
            const Locator_t& locator);

    //! Checks if the locator has WAN address (as in RTCP protocol)
    FASTDDS_EXPORTED_API static bool hasWan(
            const Locator_t& locator);

    //! Retrieves a string representation of the locator's WAN address (as in RTCP protocol)
    FASTDDS_EXPORTED_API static std::string toWanstring(
            const Locator_t& locator);

    //! This method is useful in the case of having a tcp client with an initial peer
    //! pointing to a WAN locator, and receiving a locator with LAN and WAN
    //! addresses (TCP Client from TCP Server)
    FASTDDS_EXPORTED_API static Locator_t WanToLanLocator(
            const Locator_t& locator);

    //! Sets locator's LAN ID (as in RTCP protocol)
    FASTDDS_EXPORTED_API static bool setLanID(
            Locator_t& locator,
            const std::string& lanId);

    //! Gets locator's LAN ID (as in RTCP protocol)
    FASTDDS_EXPORTED_API static const octet* getLanID(
            const Locator_t& locator);

    //! Retrieves a string representation of the locator's LAN ID (as in RTCP protocol)
    FASTDDS_EXPORTED_API static std::string toLanIDstring(
            const Locator_t& locator);

    //! Returns a new locator without logical port (as in RTCP protocol).
    FASTDDS_EXPORTED_API static Locator_t toPhysicalLocator(
            const Locator_t& locator);

    //! Checks if a locator WAN address and IP address are the same (as in RTCP protocol).
    FASTDDS_EXPORTED_API static bool ip_equals_wan(
            const Locator_t& locator);

    // Common
    //! Sets locator's RTCP port. Physical for UDP and logical for TCP (as in RTCP protocol)
    FASTDDS_EXPORTED_API static bool setPortRTPS(
            Locator_t& locator,
            uint16_t port);

    //! Gets locator's RTCP port. Physical for UDP and logical for TCP (as in RTCP protocol)
    FASTDDS_EXPORTED_API static uint16_t getPortRTPS(
            Locator_t& locator);

    //! Checks if a locator has local IP address.
    FASTDDS_EXPORTED_API static bool isLocal(
            const Locator_t& locator);

    //! Checks if a locator has any IP address.
    FASTDDS_EXPORTED_API static bool isAny(
            const Locator_t& locator);

    //! Checks if both locators has the same IP address.
    FASTDDS_EXPORTED_API static bool compareAddress(
            const Locator_t& loc1,
            const Locator_t& loc2,
            bool fullAddress = false);

    /**
     * Copies the whole address from one locator to another.
     * @param loc1 Locator to copy from.
     * @param loc2 Locator to copy to.
     * @return True if the copy was successful.
     */
    FASTDDS_EXPORTED_API static bool copy_address(
            const Locator_t& loc1,
            Locator_t& loc2);

    //! Checks if a both locators has the same IP address and physical port  (as in RTCP protocol).
    FASTDDS_EXPORTED_API static bool compareAddressAndPhysicalPort(
            const Locator_t& loc1,
            const Locator_t& loc2);

    //! Returns a string representation of the given locator.
    FASTDDS_EXPORTED_API static std::string to_string(
            const Locator_t& locator);

    // UDP
    //! Checks if the locator has a multicast IP address.
    FASTDDS_EXPORTED_API static bool isMulticast(
            const Locator_t& locator);

    //! Resolve an address name by a DNS request and return the IP that this address references by a DNS server
    FASTDDS_EXPORTED_API static std::pair<std::set<std::string>, std::set<std::string>> resolveNameDNS(
            const std::string& address_name);

    //! Check whether a string contains an IPv4 format
    FASTDDS_EXPORTED_API static bool isIPv4(
            const std::string& address);
    //! Check whether a string contains an IPv6 format
    FASTDDS_EXPORTED_API static bool isIPv6(
            const std::string& address);

protected:

    // Checks if the locator address is equal to 0
    // It checks the proper locator address depending on the locator kind
    static bool isEmpty(
            const Locator_t& locator);

    // Checks if the locator address from index till the end is equal to 0
    static bool isEmpty(
            const Locator_t& locator,
            uint16_t index);

    // Checks if a string matches an ipv6 address
    static bool IPv6isCorrect(
            const std::string& ipv6);

private:

    IPLocator() = delete;
    ~IPLocator() = delete;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_UTILS__IPLOCATOR_HPP
