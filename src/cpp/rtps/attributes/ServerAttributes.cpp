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
 * @file ServerAttributes.cpp
 *
 */

#include <forward_list>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>

#include <rtps/attributes/ServerAttributes.hpp>
#include <utils/SystemInfo.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

template<> const char* server_ostream_separators<char>::list_separator = "; ";
template<> const wchar_t* server_ostream_separators<wchar_t>::list_separator = L"; ";

template<> const char* server_ostream_separators<char>::locator_separator = "|";
template<> const wchar_t* server_ostream_separators<wchar_t>::locator_separator = L"|";

bool ros_super_client_env()
{
    std::string super_client_str;
    bool super_client = false;
    std::vector<std::string> true_vec = {"TRUE", "true", "True", "1"};
    std::vector<std::string> false_vec = {"FALSE", "false", "False", "0"};

    SystemInfo::get_env(ROS_SUPER_CLIENT, super_client_str);
    if (super_client_str != "")
    {
        if (find(true_vec.begin(), true_vec.end(), super_client_str) != true_vec.end())
        {
            super_client = true;
        }
        else if (find(false_vec.begin(), false_vec.end(), super_client_str) != false_vec.end())
        {
            super_client = false;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_PDP,
                    "Invalid value for ROS_SUPER_CLIENT environment variable : " << super_client_str);
        }
    }
    return super_client;
}

const std::string& ros_discovery_server_env()
{
    static std::string servers;
    SystemInfo::get_env(DEFAULT_ROS2_MASTER_URI, servers);
    return servers;
}

const std::string& ros_easy_mode_env()
{
    static std::string ip_value;
    SystemInfo::get_env(ROS2_EASY_MODE_URI, ip_value);

    if (!ip_value.empty())
    {
        // Check that the value is a valid IPv4 address
        if (!IPLocator::isIPv4(ip_value))
        {
            EPROSIMA_LOG_WARNING(
                SERVERATTRIBUTES,
                "Invalid format: Easy Mode IP must be a valid IPv4 address. "
                "Ignoring " << ROS2_EASY_MODE_URI << " value.");

            ip_value = "";
        }
    }
    return ip_value;
}

bool load_environment_server_info(
        LocatorList_t& servers_list)
{
    return load_environment_server_info(ros_discovery_server_env(), servers_list);
}

bool load_environment_server_info(
        const std::string& list,
        LocatorList_t& servers_list)
{
    servers_list.clear();
    if (list.empty())
    {
        return true;
    }

    /* Parsing ancillary regex
     * Addresses should be ; separated. IPLocator functions are used to identify them in the order:
     * IPv4, IPv6 or try dns resolution.
     **/
    const static std::regex ROS2_SERVER_LIST_PATTERN(R"(([^;]*);?)");
    const static std::regex ROS2_IPV4_ADDRESSPORT_PATTERN(R"(^((?:[0-9]{1,3}\.){3}[0-9]{1,3})?:?(?:(\d+))?$)");
    const static std::regex ROS2_IPV6_ADDRESSPORT_PATTERN(
        R"(^\[?((?:[0-9a-fA-F]{0,4}\:){0,7}[0-9a-fA-F]{0,4})?(?:\])?:?(?:(\d+))?$)");
    // Regex to handle DNS and UDPv4/6 expressions
    const static std::regex ROS2_DNS_DOMAINPORT_PATTERN(R"(^(UDPv[46]?:\[[\w\.:-]{0,63}\]|[\w\.-]{0,63}):?(?:(\d+))?$)");
    // Regex to handle TCPv4/6 expressions
    const static std::regex ROS2_DNS_DOMAINPORT_PATTERN_TCP(
        R"(^(TCPv[46]?:\[[\w\.:-]{0,63}\]):?(?:(\d+))?$)");

    // Filling port info
    auto process_port = [](int port, Locator_t& server)
            {
                if (port > std::numeric_limits<uint16_t>::max())
                {
                    throw std::out_of_range("Too large port passed into the server's list");
                }

                if (port < 420)
                {
                    // This is a domain id, not a port. Translate it to a port
                    PortParameters port_params;
                    uint16_t port_from_domain = port_params.get_discovery_server_port(port);
                    port = port_from_domain;
                }

                if (!IPLocator::setPhysicalPort(server, static_cast<uint16_t>(port)))
                {
                    std::stringstream ss;
                    ss << "Wrong port passed into the server's list " << port;
                    throw std::invalid_argument(ss.str());
                }
            };

    // Add new server
    auto add_server2qos = [](std::forward_list<Locator>&& locators, LocatorList_t& servers_list)
            {
                for (auto& server_address : locators)
                {
                    servers_list.push_back(std::move(server_address));
                }
            };

    try
    {
        // Do the parsing and populate the list
        Locator_t server_locator(LOCATOR_KIND_UDPv4, DEFAULT_ROS2_SERVER_PORT);

        std::sregex_iterator server_it(
            list.begin(),
            list.end(),
            ROS2_SERVER_LIST_PATTERN,
            std::regex_constants::match_not_null);

        while (server_it != std::sregex_iterator())
        {
            // Retrieve the address (IPv4, IPv6 or DNS name)
            const std::smatch::value_type sm = *++(server_it->cbegin());

            if (sm.matched)
            {
                // Now we must parse the inner expression
                std::smatch mr;
                std::string locator(sm);

                if (locator.empty())
                {
                    // It's intentionally empty to hint us to ignore this server
                }
                // Try first with IPv4
                else if (std::regex_match(locator, mr, ROS2_IPV4_ADDRESSPORT_PATTERN,
                        std::regex_constants::match_not_null))
                {
                    std::smatch::iterator it = mr.cbegin();

                    // Traverse submatches
                    if (++it != mr.cend())
                    {
                        std::string address = it->str();
                        server_locator.kind = LOCATOR_KIND_UDPv4;
                        server_locator.set_Invalid_Address();

                        if (!IPLocator::setIPv4(server_locator, address))
                        {
                            std::stringstream ss;
                            ss << "Wrong ipv4 address passed into the server's list " << address;
                            throw std::invalid_argument(ss.str());
                        }

                        if (IPLocator::isAny(server_locator))
                        {
                            // A server cannot be reach in all interfaces, it's clearly a localhost call
                            IPLocator::setIPv4(server_locator, "127.0.0.1");
                        }

                        // Get port if any
                        int port = DEFAULT_ROS2_SERVER_PORT;
                        if (++it != mr.cend() && it->matched)
                        {
                            port = stoi(it->str());
                        }

                        process_port(port, server_locator);
                    }

                    // Add server to the list
                    add_server2qos(std::forward_list<Locator>{server_locator}, servers_list);
                }
                // Try IPv6 next
                else if (std::regex_match(locator, mr, ROS2_IPV6_ADDRESSPORT_PATTERN,
                        std::regex_constants::match_not_null))
                {
                    std::smatch::iterator it = mr.cbegin();

                    // Traverse submatches
                    if (++it != mr.cend())
                    {
                        std::string address = it->str();
                        server_locator.kind = LOCATOR_KIND_UDPv6;
                        server_locator.set_Invalid_Address();

                        if (!IPLocator::setIPv6(server_locator, address))
                        {
                            std::stringstream ss;
                            ss << "Wrong ipv6 address passed into the server's list " << address;
                            throw std::invalid_argument(ss.str());
                        }

                        if (IPLocator::isAny(server_locator))
                        {
                            // A server cannot be reach in all interfaces, it's clearly a localhost call
                            IPLocator::setIPv6(server_locator, "::1");
                        }

                        // Get port if any
                        int port = DEFAULT_ROS2_SERVER_PORT;
                        if (++it != mr.cend() && it->matched)
                        {
                            port = stoi(it->str());
                        }

                        process_port(port, server_locator);
                    }

                    // Add server to the list
                    add_server2qos(std::forward_list<Locator>{server_locator}, servers_list);
                }
                // Try to resolve DNS
                else if (std::regex_match(locator, mr, ROS2_DNS_DOMAINPORT_PATTERN,
                        std::regex_constants::match_not_null))
                {
                    std::forward_list<Locator> flist;

                    {
                        std::stringstream new_locator(locator,
                                std::ios_base::in |
                                std::ios_base::out |
                                std::ios_base::ate);

                        // First try the formal notation, add default port if necessary
                        if (!mr[2].matched)
                        {
                            new_locator << ":" << DEFAULT_ROS2_SERVER_PORT;
                        }

                        new_locator >> server_locator;
                    }

                    // Otherwise add all resolved locators
                    switch ( server_locator.kind )
                    {
                        case LOCATOR_KIND_UDPv4:
                        case LOCATOR_KIND_UDPv6:
                            flist.push_front(server_locator);
                            break;
                        case LOCATOR_KIND_INVALID:
                        {
                            std::smatch::iterator it = mr.cbegin();

                            // Traverse submatches
                            if (++it != mr.cend())
                            {
                                std::string domain_name = it->str();
                                std::set<std::string> ipv4, ipv6;
                                std::tie(ipv4, ipv6) = IPLocator::resolveNameDNS(domain_name);

                                // Get port if any
                                int port = DEFAULT_ROS2_SERVER_PORT;
                                if (++it != mr.cend() && it->matched)
                                {
                                    port = stoi(it->str());
                                }

                                for ( const std::string& loc : ipv4 )
                                {
                                    server_locator.kind = LOCATOR_KIND_UDPv4;
                                    server_locator.set_Invalid_Address();
                                    IPLocator::setIPv4(server_locator, loc);

                                    if (IPLocator::isAny(server_locator))
                                    {
                                        // A server cannot be reach in all interfaces, it's clearly a localhost call
                                        IPLocator::setIPv4(server_locator, "127.0.0.1");
                                    }

                                    process_port(port, server_locator);
                                    flist.push_front(server_locator);
                                }

                                for ( const std::string& loc : ipv6 )
                                {
                                    server_locator.kind = LOCATOR_KIND_UDPv6;
                                    server_locator.set_Invalid_Address();
                                    IPLocator::setIPv6(server_locator, loc);

                                    if (IPLocator::isAny(server_locator))
                                    {
                                        // A server cannot be reach in all interfaces, it's clearly a localhost call
                                        IPLocator::setIPv6(server_locator, "::1");
                                    }

                                    process_port(port, server_locator);
                                    flist.push_front(server_locator);
                                }
                            }
                        }
                    }

                    if (flist.empty())
                    {
                        std::stringstream ss;
                        ss << "Wrong domain name passed into the server's list " << locator;
                        throw std::invalid_argument(ss.str());
                    }

                    // Add server to the list
                    add_server2qos(std::move(flist), servers_list);
                }
                // Try to resolve TCP DNS
                else if (std::regex_match(locator, mr, ROS2_DNS_DOMAINPORT_PATTERN_TCP,
                        std::regex_constants::match_not_null))
                {
                    std::forward_list<Locator> flist;

                    {
                        std::stringstream new_locator(locator,
                                std::ios_base::in |
                                std::ios_base::out |
                                std::ios_base::ate);

                        // First try the formal notation, add default port if necessary
                        if (!mr[2].matched)
                        {
                            new_locator << ":" << DEFAULT_TCP_SERVER_PORT;
                        }

                        new_locator >> server_locator;
                    }

                    // Otherwise add all resolved locators
                    switch ( server_locator.kind )
                    {
                        case LOCATOR_KIND_TCPv4:
                        case LOCATOR_KIND_TCPv6:
                            IPLocator::setLogicalPort(server_locator, IPLocator::getPhysicalPort(server_locator));
                            flist.push_front(server_locator);
                            break;
                        case LOCATOR_KIND_INVALID:
                        {
                            std::smatch::iterator it = mr.cbegin();

                            // Traverse submatches
                            if (++it != mr.cend())
                            {
                                std::string domain_name = it->str();
                                std::set<std::string> ipv4, ipv6;
                                std::tie(ipv4, ipv6) = IPLocator::resolveNameDNS(domain_name);

                                // Get port if any
                                int port = DEFAULT_TCP_SERVER_PORT;
                                if (++it != mr.cend() && it->matched)
                                {
                                    port = stoi(it->str());
                                }

                                for ( const std::string& loc : ipv4 )
                                {
                                    server_locator.kind = LOCATOR_KIND_TCPv4;
                                    server_locator.set_Invalid_Address();
                                    IPLocator::setIPv4(server_locator, loc);

                                    if (IPLocator::isAny(server_locator))
                                    {
                                        // A server cannot be reach in all interfaces, it's clearly a localhost call
                                        IPLocator::setIPv4(server_locator, "127.0.0.1");
                                    }

                                    process_port(port, server_locator);
                                    IPLocator::setLogicalPort(server_locator, static_cast<uint16_t>(port));
                                    flist.push_front(server_locator);
                                }

                                for ( const std::string& loc : ipv6 )
                                {
                                    server_locator.kind = LOCATOR_KIND_TCPv6;
                                    server_locator.set_Invalid_Address();
                                    IPLocator::setIPv6(server_locator, loc);

                                    if (IPLocator::isAny(server_locator))
                                    {
                                        // A server cannot be reach in all interfaces, it's clearly a localhost call
                                        IPLocator::setIPv6(server_locator, "::1");
                                    }

                                    process_port(port, server_locator);
                                    IPLocator::setLogicalPort(server_locator, static_cast<uint16_t>(port));
                                    flist.push_front(server_locator);
                                }
                            }
                        }
                    }

                    if (flist.empty())
                    {
                        std::stringstream ss;
                        ss << "Wrong domain name passed into the server's list " << locator;
                        throw std::invalid_argument(ss.str());
                    }

                    // Add server to the list
                    add_server2qos(std::move(flist), servers_list);
                }
                else
                {
                    std::stringstream ss;
                    ss << "Wrong locator passed into the server's list " << locator;
                    throw std::invalid_argument(ss.str());
                }
            }

            // Advance to the next server if any
            ++server_it;
        }

        // Check for server info
        if (servers_list.empty())
        {
            throw std::invalid_argument("No default server locators were provided.");
        }
    }
    catch (std::exception& e)
    {
        EPROSIMA_LOG_ERROR(SERVER_CLIENT_DISCOVERY, e.what());
        servers_list.clear();
        return false;
    }

    return true;
}

GUID_t RemoteServerAttributes::GetParticipant() const
{
    return GUID_t(guidPrefix, c_EntityId_RTPSParticipant);
}

GUID_t RemoteServerAttributes::GetPDPReader() const
{
    return GUID_t(guidPrefix, c_EntityId_SPDPReader);
}

GUID_t RemoteServerAttributes::GetPDPWriter() const
{
    return GUID_t(guidPrefix, c_EntityId_SPDPWriter);
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
