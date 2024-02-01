// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file NetmaskFilterUtils.cpp
 */

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include <fastdds/rtps/attributes/ExternalLocators.hpp>
#include <fastdds/rtps/transport/NetmaskFilterKind.h>

#include <rtps/network/NetmaskFilterUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace NetmaskFilterUtils {

NetmaskFilterKind string_to_netmask_filter_kind(
        const std::string& netmask_filter_str)
{
    static const std::unordered_map<std::string, NetmaskFilterKind> conversion_map =
    {
        {"OFF", NetmaskFilterKind::OFF},
        {"AUTO", NetmaskFilterKind::AUTO},
        {"ON", NetmaskFilterKind::ON}
    };

    auto it = conversion_map.find(netmask_filter_str);
    if (it != conversion_map.end())
    {
        return it->second;
    }
    else
    {
        std::stringstream error_msg;
        error_msg << "Failed to convert " << netmask_filter_str << " string to NetmaskFilterKind";
        throw std::invalid_argument(error_msg.str());
    }
}

bool validate_and_transform(
        NetmaskFilterKind& contained_netmask_filter,
        const NetmaskFilterKind& container_netmask_filter)
{
    if (contained_netmask_filter == NetmaskFilterKind::AUTO)
    {
        contained_netmask_filter = container_netmask_filter;
        return true;
    }
    else
    {
        if (container_netmask_filter == NetmaskFilterKind::AUTO)
        {
            return true;
        }
        else
        {
            return container_netmask_filter == contained_netmask_filter;
        }
    }
}

bool address_matches(
        const uint8_t* addr1,
        const uint8_t* addr2,
        uint64_t num_bits)
{
    uint64_t full_bytes = num_bits / 8;
    if ((0 == full_bytes) || std::equal(addr1, addr1 + full_bytes, addr2))
    {
        uint64_t rem_bits = num_bits % 8;
        if (rem_bits == 0)
        {
            return true;
        }

        uint8_t mask = 0xFF << (8 - rem_bits);
        return (addr1[full_bytes] & mask) == (addr2[full_bytes] & mask);
    }

    return false;
}

bool check_preconditions(
        const std::vector<TransportNetmaskFilterInfo>& factory_netmask_filter_info,
        bool ignore_non_matching_locators,
        std::string& error_msg)
{
    for (const TransportNetmaskFilterInfo& transport_netmask_filter_info : factory_netmask_filter_info)
    {
        const NetmaskFilterInfo& netmask_filter_info = transport_netmask_filter_info.second;
        const fastdds::rtps::NetmaskFilterKind& netmask_filter = netmask_filter_info.first;
        const std::vector<std::pair<LocatorWithMask, NetmaskFilterKind>>& allowlist = netmask_filter_info.second;

        if (netmask_filter == fastdds::rtps::NetmaskFilterKind::ON && allowlist.empty() &&
                !ignore_non_matching_locators)
        {
            std::stringstream ss;
            ss << "Invalid netmask filter configuration: netmask filter set to ON with empty allowlist"
               << " and ignore_non_matching_locators set to false."
               << " Enable ignore_non_matching_locators or explicitly set an allowlist.";
            error_msg = ss.str();
            return false;
        }
    }
    return true;
}

bool check_preconditions(
        const std::vector<TransportNetmaskFilterInfo>& factory_netmask_filter_info,
        const ExternalLocators& external_locators,
        std::string& error_msg)
{
    for (const auto& externality_item : external_locators)
    {
        if (externality_item.first > 0)
        {
            for (const auto& cost_item : externality_item.second)
            {
                for (const auto& locator : cost_item.second)
                {
                    bool locator_filtered = false;
                    for (const TransportNetmaskFilterInfo& transport_netmask_filter_info : factory_netmask_filter_info)
                    {
                        const int32_t& transport_kind = transport_netmask_filter_info.first;
                        const NetmaskFilterInfo& netmask_filter_info = transport_netmask_filter_info.second;
                        const std::vector<std::pair<LocatorWithMask,
                                NetmaskFilterKind>>& allowlist = netmask_filter_info.second;

                        if (locator.kind != transport_kind)
                        {
                            continue;
                        }

                        if (allowlist.empty())
                        {
                            return true;
                        }

                        for (const auto& allowed_interface : allowlist)
                        {
                            if (allowed_interface.second != fastdds::rtps::NetmaskFilterKind::ON ||
                                    allowed_interface.first.matches(locator))
                            {
                                return true;
                            }
                        }
                        locator_filtered = true;
                    }
                    if (locator_filtered)
                    {
                        std::stringstream ss;
                        ss << "Invalid netmask filter configuration: no data will be sent to external locators"
                           << " in same network as " << locator << " , no match found in allowlist."
                           << " Disable netmask filtering in at least one allowlist entry.";
                        error_msg = ss.str();
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

} // namespace NetmaskFilterUtils
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
