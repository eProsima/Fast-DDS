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

#include <rtps/transport/SocketTransportInterface.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastrtps/utils/IPLocator.h>

#include <rtps/network/utils/netmask_filter.hpp>

#include <utils/SystemInfo.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using IPLocator = fastrtps::rtps::IPLocator;
using IPFinder = fastrtps::rtps::IPFinder;

SocketTransportInterface::SocketTransportInterface(
        int32_t transport_kind)
    : TransportInterface(transport_kind)
    , netmask_filter_(NetmaskFilterKind::AUTO)
{
    switch (transport_kind)
    {
        case LOCATOR_KIND_UDPv4:
        case LOCATOR_KIND_TCPv4:
        {
            socket_kind_ = SocketKind::IPV4;
            break;
        }

        case LOCATOR_KIND_UDPv6:
        case LOCATOR_KIND_TCPv6:
        {
            socket_kind_ = SocketKind::IPV6;
            break;
        }

        default:
        {
            // TODO: throw exception? might break compatibility downwards
            assert(false);
            break;
        }
    }
}

SocketTransportInterface::SocketTransportInterface(
        int32_t transport_kind,
        const SocketTransportDescriptor& descriptor)
    : SocketTransportInterface(transport_kind)
{
    // Copy descriptor's netmask filter configuration
    // NOTE: participant's netmask_filter already taken into account before calling tranport registration
    netmask_filter_ = descriptor.netmask_filter;

    if (!descriptor.interfaceWhiteList.empty() || !descriptor.interface_allowlist.empty() ||
            !descriptor.interface_blocklist.empty())
    {
        const auto white_begin = descriptor.interfaceWhiteList.begin();
        const auto white_end = descriptor.interfaceWhiteList.end();

        const auto allow_begin = descriptor.interface_allowlist.begin();
        const auto allow_end = descriptor.interface_allowlist.end();

        const auto block_begin = descriptor.interface_blocklist.begin();
        const auto block_end = descriptor.interface_blocklist.end();

        if (!descriptor.interfaceWhiteList.empty())
        {
            EPROSIMA_LOG_WARNING(TRANSPORT_SOCKET,
                    "Support for interfaceWhiteList will be removed in a future release."
                    << " Please use interface allowlist/blocklist instead.");
        }

        std::vector<IPFinder::info_IP> local_interfaces;
        get_ips(local_interfaces, true, false);
        for (const IPFinder::info_IP& infoIP : local_interfaces)
        {
            if (std::find_if(block_begin, block_end, [this, infoIP](const BlockedNetworkInterface& blocklist_element)
                    {
                        return blocklist_element.name == infoIP.dev || compare_ips(blocklist_element.name, infoIP.name);
                    }) != block_end )
            {
                // Before skipping this interface, check if present in whitelist/allowlist and warn the user if found
                if ((std::find_if(white_begin, white_end, [this, infoIP](const std::string& whitelist_element)
                        {
                            return whitelist_element == infoIP.dev || compare_ips(whitelist_element, infoIP.name);
                        }) != white_end ) ||
                        (std::find_if(allow_begin, allow_end,
                        [this, infoIP](const AllowedNetworkInterface& allowlist_element)
                        {
                            return allowlist_element.name == infoIP.dev || compare_ips(allowlist_element.name, infoIP.name);
                        }) != allow_end ))
                {
                    EPROSIMA_LOG_WARNING(TRANSPORT_SOCKET,
                            "Blocked interface " << infoIP.dev << ": " << infoIP.name
                                                 << " is also present in whitelist/allowlist."
                                                 << " Blocklist takes precedence over whitelist/allowlist.");
                }
                continue;
            }
            else if (descriptor.interfaceWhiteList.empty() && descriptor.interface_allowlist.empty())
            {
                allowed_interfaces_.emplace_back(infoIP.dev, infoIP.name, infoIP.masked_locator, descriptor.netmask_filter);
            }
            else if (!descriptor.interface_allowlist.empty())
            {
                auto allow_it = std::find_if(
                    allow_begin,
                    allow_end,
                    [this, &infoIP](const AllowedNetworkInterface& allowlist_element)
                    {
                        return allowlist_element.name == infoIP.dev || compare_ips(allowlist_element.name, infoIP.name);
                    });
                if (allow_it != allow_end)
                {
                    NetmaskFilterKind netmask_filter = allow_it->netmask_filter;
                    if (network::netmask_filter::validate_and_transform(netmask_filter,
                            descriptor.netmask_filter))
                    {
                        allowed_interfaces_.emplace_back(infoIP.dev, infoIP.name, infoIP.masked_locator, netmask_filter);
                    }
                    else
                    {
                        EPROSIMA_LOG_WARNING(TRANSPORT_SOCKET,
                                "Ignoring allowed interface " << infoIP.dev << ": " << infoIP.name
                                                              << " as its netmask filter configuration (" << netmask_filter << ") is incompatible"
                                                              << " with descriptor's (" << descriptor.netmask_filter <<
                                ").");
                    }
                }
            }
            else if (!descriptor.interfaceWhiteList.empty())
            {
                if (std::find_if(white_begin, white_end, [this, infoIP](const std::string& whitelist_element)
                        {
                            return whitelist_element == infoIP.dev || compare_ips(whitelist_element, infoIP.name);
                        }) != white_end )
                {
                    allowed_interfaces_.emplace_back(infoIP.dev, infoIP.name, infoIP.masked_locator, descriptor.netmask_filter);
                }
            }
        }

        if (allowed_interfaces_.empty())
        {
            EPROSIMA_LOG_ERROR(TRANSPORT_SOCKET, "All whitelist interfaces were filtered out");
            allowed_interfaces_.emplace_back("dummy_iface");
        }
    }
}

SocketTransportInterface::~SocketTransportInterface()
{
}

bool SocketTransportInterface::is_ipv4() const
{
    switch (transport_kind_)
    {
        case LOCATOR_KIND_UDPv4:
        case LOCATOR_KIND_TCPv4:
        {
            return true;
        }

        case LOCATOR_KIND_UDPv6:
        case LOCATOR_KIND_TCPv6:
        {
            return false;
        }

        default:
        {
            // TODO: throw exception?
            assert(false);
            return false;
        }
    }
}

bool SocketTransportInterface::IsLocatorSupported(
        const Locator& locator) const
{
    return locator.kind == transport_kind_;
}

LocatorList SocketTransportInterface::NormalizeLocator(
        const Locator& locator)
{
    LocatorList list;
    if (IPLocator::isAny(locator))
    {
        std::vector<IPFinder::info_IP> locNames;
        get_ips(locNames, false, false);
        for (const auto& infoIP : locNames)
        {
            if (is_interface_allowed(infoIP.name))
            {
                Locator newloc(locator);
                is_ipv4() ? IPLocator::setIPv4(newloc, infoIP.locator) : IPLocator::setIPv6(newloc, infoIP.locator);
                list.push_back(newloc);
            }
        }
        if (list.empty())
        {
            Locator newloc(locator);
            is_ipv4() ? IPLocator::setIPv4(newloc, "127.0.0.1") : IPLocator::setIPv6(newloc, "::1");
            list.push_back(newloc);
        }
    }
    else
    {
        list.push_back(locator);
    }

    return list;
}

bool SocketTransportInterface::is_local_locator(
        const Locator& locator) const
{
    assert(locator.kind == kind());

    if (IPLocator::isLocal(locator))
    {
        return true;
    }

    std::vector<IPFinder::info_IP> currentInterfaces;
    if (!get_ips(currentInterfaces, false, false))
    {
        EPROSIMA_LOG_WARNING(TRANSPORT_SOCKET,
                "Could not retrieve IPs information to check if locator " << locator << " is local.");
        return false;
    }
    for (const IPFinder::info_IP& localInterface : currentInterfaces)
    {
        if (IPLocator::compareAddress(locator, localInterface.locator))
        {
            return true;
        }
    }

    return false;
}

bool SocketTransportInterface::is_localhost_allowed() const
{
    Locator local_locator;
    fill_local_ip(local_locator);
    return is_locator_allowed(local_locator);
}


bool SocketTransportInterface::transform_remote_locator(
        const Locator& remote_locator,
        Locator& result_locator,
        bool allowed_remote_localhost,
        bool allowed_local_localhost) const
{
    if (IsLocatorSupported(remote_locator))
    {
        result_locator = remote_locator;
        if (!is_local_locator(result_locator))
        {
            // is_local_locator will return false for multicast addresses as well as remote unicast ones.
            return true;
        }

        // If we get here, the locator is a local unicast address

        // Attempt conversion to localhost if remote transport listening on it allows it
        if (allowed_remote_localhost)
        {
            Locator loopbackLocator;
            fill_local_ip(loopbackLocator);
            if (is_locator_allowed(loopbackLocator))
            {
                // Locator localhost is in the whitelist, so use localhost instead of remote_locator
                fill_local_ip(result_locator);
                result_locator.port = remote_locator.port;
                return true;
            }
            else if (allowed_local_localhost)
            {
                // Abort transformation if localhost not allowed by this transport, but it is by other local transport
                // and the remote one.
                return false;
            }
        }

        if (!is_locator_allowed(result_locator))
        {
            // Neither original remote locator nor localhost allowed: abort.
            return false;
        }

        return true;
    }
    return false;
}

NetmaskFilterInfo SocketTransportInterface::netmask_filter_info() const
{
    return {netmask_filter_, allowed_interfaces_};
}

bool SocketTransportInterface::compare_locator_ip(
        const Locator& lh,
        const Locator& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool SocketTransportInterface::compare_locator_ip_and_port(
        const Locator& lh,
        const Locator& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void SocketTransportInterface::fill_local_ip(
        Locator& loc) const
{
    loc.kind = kind();
    is_ipv4() ? IPLocator::setIPv4(loc, "127.0.0.1") : IPLocator::setIPv6(loc, "::1");
}

bool SocketTransportInterface::get_ips(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback,
        bool force_lookup) const
{
    if (!SystemInfo::get_ips(locNames, return_loopback, force_lookup))
    {
        return false;
    }

    // Keep only relevant IPs
    auto remove_ip = [this](IPFinder::info_IP ip)
                        {
                            return is_ipv4() ? (ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL) : (ip.type != IPFinder::IP6 && ip.type != IPFinder::IP6_LOCAL);
                        };
    auto new_end = remove_if(locNames.begin(),
                            locNames.end(),
                            remove_ip);
    locNames.erase(new_end, locNames.end());

    // Assign transport kind to retrieved locators
    std::for_each(locNames.begin(), locNames.end(), [this](IPFinder::info_IP& loc)
            {
                loc.locator.kind = kind();
                loc.masked_locator.kind = kind();
            });

    return true;
}

bool SocketTransportInterface::get_ips_unique_interfaces(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback,
        bool force_lookup) const
{
    if (!get_ips(locNames, return_loopback, force_lookup))
    {
        return false;
    }
    std::sort(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool
            {
                return a.dev < b.dev;
            });
    auto new_end = std::unique(locNames.begin(), locNames.end(),
                    [this](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool
                    {
                        IPFinder::IPTYPE local_type = is_ipv4() ? IPFinder::IP4_LOCAL : IPFinder::IP6_LOCAL;
                        return a.type != local_type && b.type != local_type && a.dev == b.dev;
                    });
    locNames.erase(new_end, locNames.end());
    return true;
}

const std::string& SocketTransportInterface::localhost_name()
{
    static const std::string ip_localhost = is_ipv4() ? "127.0.0.1" : "::1";
    return ip_localhost;
}

bool SocketTransportInterface::is_interface_allowlist_empty() const
{
    return allowed_interfaces_.empty();
}

bool SocketTransportInterface::is_interface_allowed(
        const std::string& iface) const
{
    if (allowed_interfaces_.empty())
    {
        return true;
    }

    if (is_ipv4() ? (iface == s_IPv4AddressAny) : (iface == s_IPv6AddressAny))
    {
        return true;
    }

    return std::find_if(allowed_interfaces_.begin(), allowed_interfaces_.end(), [this, &iface](const AllowedNetworkInterface& allowlist_element)
            {
                return compare_ips(allowlist_element.ip, iface);
            }) != allowed_interfaces_.end();
}

bool SocketTransportInterface::is_interface_allowed(
        const Locator& loc) const
{
    return is_interface_allowed(is_ipv4() ? IPLocator::toIPv4string(loc) : IPLocator::toIPv6string(loc));
}

std::vector<std::string> SocketTransportInterface::get_binding_interfaces_list()
{
    std::vector<std::string> vOutputInterfaces;
    if (is_interface_allowlist_empty())
    {
        is_ipv4() ? vOutputInterfaces.push_back(s_IPv4AddressAny) : vOutputInterfaces.push_back(s_IPv6AddressAny);
    }
    else
    {
        for (auto& iface : allowed_interfaces_)
        {
            vOutputInterfaces.push_back(iface.ip);
        }
    }

    return vOutputInterfaces;
}

bool SocketTransportInterface::is_locator_allowed(
        const Locator& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }
    if (allowed_interfaces_.empty() || IPLocator::isMulticast(locator))
    {
        return true;
    }

    return is_interface_allowed(locator);
}

bool SocketTransportInterface::compare_ips(
        const std::string& ip1,
        const std::string& ip2) const
{
    if (is_ipv4())
    {
        return ip1 == ip2;
    }
    else
    {
        // string::find returns string::npos if the character is not found
        // If the second parameter is string::npos value, it indicates to take all characters until the end of the string
        std::string substr1 = ip1.substr(0, ip1.find('%'));
        std::string substr2 = ip2.substr(0, ip2.find('%'));

        if (substr1.compare(substr2) == 0)
        {
            return true;
        }
        return false;
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
