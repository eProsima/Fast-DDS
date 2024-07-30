// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rtps/transport/TCPv4Transport.h>

#include <utility>
#include <cstring>
#include <algorithm>

#include <asio.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <rtps/network/utils/netmask_filter.hpp>
#include <utils/SystemInfo.hpp>

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

static bool get_ipv4s(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback,
        bool force_lookup)
{
    if (!SystemInfo::get_ips(locNames, return_loopback, force_lookup))
    {
        return false;
    }

    auto new_end = remove_if(locNames.begin(),
                    locNames.end(),
                    [](IPFinder::info_IP ip)
                    {
                        return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL;
                    });
    locNames.erase(new_end, locNames.end());
    std::for_each(locNames.begin(), locNames.end(), [](IPFinder::info_IP& loc)
            {
                loc.locator.kind = LOCATOR_KIND_TCPv4;
                loc.masked_locator.kind = LOCATOR_KIND_TCPv4;
            });
    return true;
}

static asio::ip::address_v4::bytes_type locator_to_native(
        Locator& locator,
        const octet* local_wan)
{
    const octet* wan = IPLocator::getWan(locator);
    if (IPLocator::hasWan(locator) && (memcmp(local_wan, wan, 4) != 0))
    {
        return{ { wan[0], wan[1], wan[2], wan[3]} };
    }
    else
    {
        return{ { IPLocator::getIPv4(locator)[0],
            IPLocator::getIPv4(locator)[1],
            IPLocator::getIPv4(locator)[2],
            IPLocator::getIPv4(locator)[3]} };
    }
}

TCPv4Transport::TCPv4Transport(
        const TCPv4TransportDescriptor& descriptor)
    : TCPTransportInterface(LOCATOR_KIND_TCPv4)
    , configuration_(descriptor)
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
            EPROSIMA_LOG_WARNING(TRANSPORT_TCPV4,
                    "Support for interfaceWhiteList will be removed in a future release."
                    << " Please use interface allowlist/blocklist instead.");
        }

        std::vector<IPFinder::info_IP> local_interfaces;
        get_ipv4s(local_interfaces, true, false);
        for (const IPFinder::info_IP& infoIP : local_interfaces)
        {
            if (std::find_if(block_begin, block_end, [infoIP](const BlockedNetworkInterface& blocklist_element)
                    {
                        return blocklist_element.name == infoIP.dev || blocklist_element.name == infoIP.name;
                    }) != block_end )
            {
                // Before skipping this interface, check if present in whitelist/allowlist and warn the user if found
                if ((std::find_if(white_begin, white_end, [infoIP](const std::string& whitelist_element)
                        {
                            return whitelist_element == infoIP.dev || whitelist_element == infoIP.name;
                        }) != white_end ) ||
                        (std::find_if(allow_begin, allow_end,
                        [infoIP](const AllowedNetworkInterface& allowlist_element)
                        {
                            return allowlist_element.name == infoIP.dev || allowlist_element.name == infoIP.name;
                        }) != allow_end ))
                {
                    EPROSIMA_LOG_WARNING(TRANSPORT_TCPV4,
                            "Blocked interface " << infoIP.dev << ": " << infoIP.name
                                                 << " is also present in whitelist/allowlist."
                                                 << " Blocklist takes precedence over whitelist/allowlist.");
                }
                continue;
            }
            else if (descriptor.interfaceWhiteList.empty() && descriptor.interface_allowlist.empty())
            {
                interface_whitelist_.emplace_back(ip::address_v4::from_string(infoIP.name));
                allowed_interfaces_.emplace_back(infoIP.dev, infoIP.name, infoIP.masked_locator,
                        descriptor.netmask_filter);
            }
            else if (!descriptor.interface_allowlist.empty())
            {
                auto allow_it = std::find_if(
                    allow_begin,
                    allow_end,
                    [&infoIP](const AllowedNetworkInterface& allowlist_element)
                    {
                        return allowlist_element.name == infoIP.dev || allowlist_element.name == infoIP.name;
                    });
                if (allow_it != allow_end)
                {
                    NetmaskFilterKind netmask_filter = allow_it->netmask_filter;
                    if (network::netmask_filter::validate_and_transform(netmask_filter,
                            descriptor.netmask_filter))
                    {
                        interface_whitelist_.emplace_back(ip::address_v4::from_string(infoIP.name));
                        allowed_interfaces_.emplace_back(infoIP.dev, infoIP.name, infoIP.masked_locator,
                                netmask_filter);
                    }
                    else
                    {
                        EPROSIMA_LOG_WARNING(TRANSPORT_TCPV4,
                                "Ignoring allowed interface " << infoIP.dev << ": " << infoIP.name
                                                              << " as its netmask filter configuration (" << netmask_filter << ") is incompatible"
                                                              << " with descriptor's (" << descriptor.netmask_filter <<
                                ").");
                    }
                }
            }
            else if (!descriptor.interfaceWhiteList.empty())
            {
                if (std::find_if(white_begin, white_end, [infoIP](const std::string& whitelist_element)
                        {
                            return whitelist_element == infoIP.dev || whitelist_element == infoIP.name;
                        }) != white_end )
                {
                    interface_whitelist_.emplace_back(ip::address_v4::from_string(infoIP.name));
                    allowed_interfaces_.emplace_back(infoIP.dev, infoIP.name, infoIP.masked_locator,
                            descriptor.netmask_filter);
                }
            }
        }

        if (interface_whitelist_.empty())
        {
            EPROSIMA_LOG_ERROR(TRANSPORT_TCPV4, "All whitelist interfaces were filtered out");
            interface_whitelist_.emplace_back(ip::address_v4::from_string("192.0.2.0"));
        }
    }

    if (!configuration_.listening_ports.empty())
    {
        if (configuration_.listening_ports.size() > 1)
        {
            EPROSIMA_LOG_ERROR(TRANSPORT_TCPV4,
                    "Only one listening port is allowed for TCP transport. Only the first port will be used.");
            configuration_.listening_ports.erase(
                configuration_.listening_ports.begin() + 1, configuration_.listening_ports.end());
        }
        Locator locator(LOCATOR_KIND_TCPv4, configuration_.listening_ports.front());
        configuration_.listening_ports.front() = create_acceptor_socket(locator);
    }

#if !TLS_FOUND
    if (descriptor.apply_security)
    {
        EPROSIMA_LOG_ERROR(RTCP_TLSV4, "Trying to use TCP Transport with TLS but TLS was not found.");
    }
#endif // if !TLS_FOUND
}

TCPv4Transport::TCPv4Transport()
    : TCPTransportInterface(LOCATOR_KIND_TCPv4)
{
}

TCPv4Transport::~TCPv4Transport()
{
    clean();
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor()
    : TCPTransportDescriptor()
{
    memset(wan_addr, 0, 4);
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor(
        const TCPv4TransportDescriptor& t)
    : TCPTransportDescriptor(t)
{
    memcpy(wan_addr, t.wan_addr, 4);
}

TCPv4TransportDescriptor& TCPv4TransportDescriptor::operator =(
        const TCPv4TransportDescriptor& t)
{
    *static_cast<TCPTransportDescriptor*>(this) = t;
    memcpy(wan_addr, t.wan_addr, 4);

    return *this;
}

bool TCPv4TransportDescriptor::operator ==(
        const TCPv4TransportDescriptor& t) const
{
    return (this->wan_addr[0] == t.wan_addr[0] &&
           this->wan_addr[1] == t.wan_addr[1] &&
           this->wan_addr[2] == t.wan_addr[2] &&
           this->wan_addr[3] == t.wan_addr[3] &&
           TCPTransportDescriptor::operator ==(t));
}

TransportInterface* TCPv4TransportDescriptor::create_transport() const
{
    return new TCPv4Transport(*this);
}

void TCPv4Transport::AddDefaultOutputLocator(
        LocatorList&)
{
}

const TCPTransportDescriptor* TCPv4Transport::configuration() const
{
    return &configuration_;
}

TCPTransportDescriptor* TCPv4Transport::configuration()
{
    return &configuration_;
}

bool TCPv4Transport::get_ips(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback,
        bool force_lookup) const
{
    return get_ipv4s(locNames, return_loopback, force_lookup);
}

uint16_t TCPv4Transport::GetLogicalPortIncrement() const
{
    return configuration_.logical_port_increment;
}

uint16_t TCPv4Transport::GetLogicalPortRange() const
{
    return configuration_.logical_port_range;
}

uint16_t TCPv4Transport::GetMaxLogicalPort() const
{
    return configuration_.max_logical_port;
}

std::vector<std::string> TCPv4Transport::get_binding_interfaces_list()
{
    std::vector<std::string> vOutputInterfaces;
    if (is_interface_whitelist_empty())
    {
        vOutputInterfaces.push_back(s_IPv4AddressAny);
    }
    else
    {
        for (auto& ip : interface_whitelist_)
        {
            vOutputInterfaces.push_back(ip.to_string());
        }
    }

    return vOutputInterfaces;
}

bool TCPv4Transport::is_interface_whitelist_empty() const
{
    return interface_whitelist_.empty();
}

bool TCPv4Transport::is_interface_allowed(
        const std::string& iface) const
{
    return is_interface_allowed(asio::ip::address_v4::from_string(iface));
}

bool TCPv4Transport::is_interface_allowed(
        const ip::address_v4& ip) const
{
    if (interface_whitelist_.empty())
    {
        return true;
    }

    if (ip == ip::address_v4::any())
    {
        return true;
    }

    return find(interface_whitelist_.begin(), interface_whitelist_.end(), ip) != interface_whitelist_.end();
}

LocatorList TCPv4Transport::NormalizeLocator(
        const Locator& locator)
{
    LocatorList list;

    if (IPLocator::isAny(locator))
    {
        std::vector<IPFinder::info_IP> locNames;
        get_ipv4s(locNames, false, false);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v4::from_string(infoIP.name);
            if (is_interface_allowed(ip))
            {
                Locator newloc(locator);
                IPLocator::setIPv4(newloc, infoIP.locator);
                list.push_back(newloc);
            }
        }
        if (list.empty())
        {
            Locator newloc(locator);
            IPLocator::setIPv4(newloc, "127.0.0.1");
            list.push_back(newloc);
        }
    }
    else
    {
        list.push_back(locator);
    }

    return list;
}

bool TCPv4Transport::is_local_locator(
        const Locator& locator) const
{
    assert(locator.kind == LOCATOR_KIND_TCPv4);

    /*
     * Check case: Remote WAN address isn't our WAN address.
     */
    if (IPLocator::hasWan(locator))
    {
        const octet* wan = IPLocator::getWan(locator);
        if (memcmp(wan, configuration_.wan_addr, 4 * sizeof(octet)) != 0)
        {
            return false; // WAN mismatch
        }
    }

    /*
     * Check case: Address is localhost
     */
    if (IPLocator::isLocal(locator))
    {
        return true;
    }

    /*
     * Check case: Address is one of our addresses.
     */
    std::vector<IPFinder::info_IP> current_interfaces;
    if (!get_ips(current_interfaces, false, false))
    {
        EPROSIMA_LOG_WARNING(TRANSPORT_TCPV4,
                "Could not retrieve IPs information to check if locator " << locator << " is local.");
        return false;
    }
    for (const IPFinder::info_IP& localInterface : current_interfaces)
    {
        if (IPLocator::compareAddress(locator, localInterface.locator))
        {
            return true;
        }
    }

    return false;
}

bool TCPv4Transport::is_locator_allowed(
        const Locator& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }
    if (interface_whitelist_.empty())
    {
        return true;
    }
    return is_interface_allowed(IPLocator::toIPv4string(locator));
}

bool TCPv4Transport::is_locator_reachable(
        const Locator_t& locator)
{
    return IsLocatorSupported(locator);
}

bool TCPv4Transport::compare_locator_ip(
        const Locator& lh,
        const Locator& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool TCPv4Transport::compare_locator_ip_and_port(
        const Locator& lh,
        const Locator& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void TCPv4Transport::fill_local_ip(
        Locator& loc) const
{
    loc.kind = kind();
    IPLocator::setIPv4(loc, "127.0.0.1");
}

ip::tcp::endpoint TCPv4Transport::generate_endpoint(
        const Locator& loc,
        uint16_t port) const
{
    asio::ip::address_v4::bytes_type remoteAddress;
    IPLocator::copyIPv4(loc, remoteAddress.data());
    return ip::tcp::endpoint(asio::ip::address_v4(remoteAddress), port);
}

ip::tcp::endpoint TCPv4Transport::generate_local_endpoint(
        Locator& loc,
        uint16_t port) const
{
    return ip::tcp::endpoint(asio::ip::address_v4(locator_to_native(loc, configuration_.wan_addr)), port);
}

ip::tcp::endpoint TCPv4Transport::generate_endpoint(
        uint16_t port) const
{
    return asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
}

asio::ip::tcp TCPv4Transport::generate_protocol() const
{
    return asio::ip::tcp::v4();
}

bool TCPv4Transport::is_interface_allowed(
        const Locator& loc) const
{
    asio::ip::address_v4 ip = asio::ip::address_v4::from_string(IPLocator::toIPv4string(loc));
    return is_interface_allowed(ip);
}

void TCPv4Transport::set_receive_buffer_size(
        uint32_t size)
{
    configuration_.receiveBufferSize = size;
}

void TCPv4Transport::set_send_buffer_size(
        uint32_t size)
{
    configuration_.sendBufferSize = size;
}

void TCPv4Transport::endpoint_to_locator(
        const ip::tcp::endpoint& endpoint,
        Locator& locator) const
{
    locator.kind = LOCATOR_KIND_TCPv4;
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    IPLocator::setIPv4(locator, ipBytes.data());
}

bool TCPv4Transport::fillMetatrafficUnicastLocator(
        Locator& locator,
        uint32_t metatraffic_unicast_port) const
{
    bool result = TCPTransportInterface::fillMetatrafficUnicastLocator(locator, metatraffic_unicast_port);

    IPLocator::setWan(locator,
            configuration_.wan_addr[0], configuration_.wan_addr[1],
            configuration_.wan_addr[2], configuration_.wan_addr[3]);

    return result;
}

bool TCPv4Transport::fillUnicastLocator(
        Locator& locator,
        uint32_t well_known_port) const
{
    bool result = TCPTransportInterface::fillUnicastLocator(locator, well_known_port);

    IPLocator::setWan(locator,
            configuration_.wan_addr[0], configuration_.wan_addr[1],
            configuration_.wan_addr[2], configuration_.wan_addr[3]);

    return result;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
