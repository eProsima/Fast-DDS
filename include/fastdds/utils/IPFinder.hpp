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
 * @file IPFinder.hpp
 *
 */

#ifndef FASTDDS_UTILS__IPFINDER_HPP
#define FASTDDS_UTILS__IPFINDER_HPP

#include <string>
#include <vector>

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/LocatorWithMask.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
/**
 * Class IPFinder, to determine the IP of the NICs.
 * @ingroup UTILITIES_MODULE
 */
class IPFinder
{
public:

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
    /**
     * Enum IPTYPE, to define the type of IP obtained from the NICs.
     */
    enum IPTYPE
    {
        IP4,          //!< IP4
        IP6,          //!< IP6
        IP4_LOCAL,    //!< IP4_LOCAL
        IP6_LOCAL     //!< IP6_LOCAL
    };
    /**
     * Structure info_IP with information about a specific IP obtained from a NIC.
     */
    typedef struct info_IP
    {
        IPTYPE type;
        std::string name;
        std::string dev;
        Locator_t locator;
        fastdds::rtps::LocatorWithMask masked_locator;
    }info_IP;

    /**
     * Structure info_MAC with information about a specific MAC obtained from a NIC.
     */
    typedef struct info_MAC
    {
        unsigned char address[6];

        bool operator == (
                const info_MAC& other)
        {
            return memcmp(address, other.address, 6) == 0;
        }

    }info_MAC;

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
    IPFinder();
    virtual ~IPFinder();

    FASTDDS_EXPORTED_API static bool getIPs(
            std::vector<info_IP>* vec_name,
            bool return_loopback = false);

    /**
     * Get the IP4Adresses in all interfaces.
     * @param [out] locators List of locators to be populated with the IP4 addresses.
     */
    FASTDDS_EXPORTED_API static bool getIP4Address(
            LocatorList_t* locators);
    /**
     * Get the IP6Adresses in all interfaces.
     * @param [out] locators List of locators to be populated with the IP6 addresses.
     */
    FASTDDS_EXPORTED_API static bool getIP6Address(
            LocatorList_t* locators);
    /**
     * Get all IP Adresses in all interfaces.
     * @param [out] locators List of locators to be populated with the addresses.
     */
    FASTDDS_EXPORTED_API static bool getAllIPAddress(
            LocatorList_t* locators);
    /**
     * Parses an IP4 string, populating a info_IP with its value.
     * @param [out] info info_IP to populate.
     * */
    FASTDDS_EXPORTED_API static bool parseIP4(
            info_IP& info);
    /**
     * Parses an IP6 string, populating a info_IP with its value.
     * @param [out] info info_IP to populate.
     * */
    FASTDDS_EXPORTED_API static bool parseIP6(
            info_IP& info);

    FASTDDS_EXPORTED_API static std::string getIPv4Address(
            const std::string& name);
    FASTDDS_EXPORTED_API static std::string getIPv6Address(
            const std::string& name);

    /**
     * Get all MAC Adresses of all interfaces.
     * Will return all unique MAC addresses for eadh of the interfaces returned by getAllIPAddress
     * @param [out] macs List of MAC addresses.
     */
    FASTDDS_EXPORTED_API static bool getAllMACAddress(
            std::vector<info_MAC>* macs);
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_UTILS__IPFINDER_HPP
