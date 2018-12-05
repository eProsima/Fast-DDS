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
 * @file IPLocator.h
 *
 */

#ifndef IP_LOCATOR_H_
#define IP_LOCATOR_H_

#include <vector>
#include <string>

#include "../rtps/common/Locator.h"

namespace eprosima {
namespace fastrtps {
namespace rtps {
/**
 * Class IPLocator, to provide helper functions to the IP based transports.
 * @ingroup UTILITIES_MODULE
 */
class IPLocator
{
    public:
        IPLocator();
        virtual ~IPLocator();

        // Factory
        RTPS_DllAPI static void createLocator(int32_t kindin, const std::string& address, uint32_t portin, Locator_t& locator);

        // IPv4
        RTPS_DllAPI static bool setIPv4(Locator_t &locator, const unsigned char* addr);
        RTPS_DllAPI static bool setIPv4(Locator_t &locator, octet o1, octet o2, octet o3, octet o4);
        RTPS_DllAPI static bool setIPv4(Locator_t &locator, const std::string &ipv4);
        RTPS_DllAPI static bool setIPv4(Locator_t &destlocator, const Locator_t &origlocator);
        RTPS_DllAPI static const octet* getIPv4(const Locator_t &locator);
        RTPS_DllAPI static bool hasIPv4(const Locator_t &locator);
        RTPS_DllAPI static std::string toIPv4string(const Locator_t &locator);
        RTPS_DllAPI static bool copyIPv4(const Locator_t &locator, unsigned char* dest);

        // IPv6
        RTPS_DllAPI static bool setIPv6(Locator_t &locator, const unsigned char* addr);
        RTPS_DllAPI static bool setIPv6(Locator_t &locator,
                uint16_t group0, uint16_t group1, uint16_t group2, uint16_t group3,
                uint16_t group4, uint16_t group5, uint16_t group6, uint16_t group7);
        RTPS_DllAPI static bool setIPv6(Locator_t &locator, const std::string &ipv6);
        RTPS_DllAPI static bool setIPv6(Locator_t &destlocator, const Locator_t &origlocator);
        RTPS_DllAPI static const octet* getIPv6(const Locator_t &locator);
        RTPS_DllAPI static bool hasIPv6(const Locator_t &locator);
        RTPS_DllAPI static std::string toIPv6string(const Locator_t &locator);
        RTPS_DllAPI static bool copyIPv6(const Locator_t &locator, unsigned char* dest);

        // TCP
        RTPS_DllAPI static bool setLogicalPort(Locator_t &locator, uint16_t port);
        RTPS_DllAPI static uint16_t getLogicalPort(const Locator_t &locator);
        RTPS_DllAPI static bool setPhysicalPort(Locator_t &locator, uint16_t port);
        RTPS_DllAPI static uint16_t getPhysicalPort(const Locator_t &locator);

        // TCPv4
        RTPS_DllAPI static bool setWan(Locator_t &locator, octet o1, octet o2, octet o3, octet o4);
        RTPS_DllAPI static bool setWan(Locator_t &locator, const std::string &wan);
        RTPS_DllAPI static const octet* getWan(const Locator_t &locator);
        RTPS_DllAPI static bool hasWan(const Locator_t &locator);
        RTPS_DllAPI static std::string toWanstring(const Locator_t &locator);

        RTPS_DllAPI static bool setLanID(Locator_t &locator, const std::string &lanId);
        RTPS_DllAPI static const octet* getLanID(const Locator_t &locator);
        RTPS_DllAPI static std::string toLanIDstring(const Locator_t &locator);
        RTPS_DllAPI static Locator_t toPhysicalLocator(const Locator_t &locator);

        // Common
        RTPS_DllAPI static bool setPortRTPS(Locator_t &locator, uint16_t port);
        RTPS_DllAPI static uint16_t getPortRTPS(Locator_t &locator);
        RTPS_DllAPI static bool isLocal(const Locator_t &locator);
        RTPS_DllAPI static bool isAny(const Locator_t &locator);
        RTPS_DllAPI static bool compareAddress(const Locator_t &loc1, const Locator_t &loc2, bool fullAddress = false);
        RTPS_DllAPI static bool compareAddressAndPhysicalPort(const Locator_t &loc1, const Locator_t &loc2);

        // UDP
        RTPS_DllAPI static bool isMulticast(const Locator_t &locator);
};

}
}
} /* namespace eprosima */

#endif /* IP_LOCATOR_H_ */
