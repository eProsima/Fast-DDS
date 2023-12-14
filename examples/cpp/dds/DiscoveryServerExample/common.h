// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file common.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_COMMON_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_COMMON_H_

#include <fastdds/rtps/attributes/ServerAttributes.h>
#include <fastrtps/utils/IPLocator.h>

enum class TransportKind
{
    UDPv4,
    UDPv6,
    TCPv4,
    TCPv6,
    SHM,
};

inline eprosima::fastrtps::rtps::GuidPrefix_t get_discovery_server_guid_from_id(
        unsigned short id)
{
    eprosima::fastrtps::rtps::GuidPrefix_t result;

    // Get default DS guid and modify the one value expected to be changed
    std::istringstream(eprosima::fastdds::rtps::DEFAULT_ROS2_SERVER_GUIDPREFIX) >> result;
    result.value[2] =
            static_cast<eprosima::fastrtps::rtps::octet>(id); // This is done like this in Fast

    return result;
}

inline bool is_ip(
        const std::string ip_str)
{
    return eprosima::fastrtps::rtps::IPLocator::isIPv4(ip_str) || eprosima::fastrtps::rtps::IPLocator::isIPv6(ip_str);
}

inline std::string get_ip_from_dns(
        const std::string& domain_name,
        TransportKind kind)
{
    std::pair<std::set<std::string>, std::set<std::string>> dns_response =
            eprosima::fastrtps::rtps::IPLocator::resolveNameDNS(domain_name);

    if (kind == TransportKind::UDPv4 || kind == TransportKind::TCPv4)
    {
        if (dns_response.first.empty())
        {
            std::cout << "Not DNS found for IPv4 for " << domain_name << std::endl;
            return "";
        }
        else
        {
            std::string solution(*dns_response.first.begin());
            std::cout << "DNS found for " << domain_name << " => " << solution << std::endl;
            return solution;
        }
    }
    else if (kind == TransportKind::UDPv6 || kind == TransportKind::TCPv6)
    {
        if (dns_response.second.empty())
        {
            std::cout << "Not DNS found for IPv6 for " << domain_name << std::endl;
            return "";
        }
        else
        {
            std::string solution(*dns_response.second.begin());
            std::cout << "DNS found for " << domain_name << " => " << solution << std::endl;
            return solution;
        }
    }

    return domain_name;
}

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_COMMON_H_ */
