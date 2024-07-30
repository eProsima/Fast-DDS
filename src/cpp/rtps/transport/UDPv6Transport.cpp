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

#include <rtps/transport/UDPv6Transport.h>

#include <algorithm>
#include <cstring>
#include <utility>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <rtps/messages/MessageReceiver.h>
#include <rtps/network/utils/netmask_filter.hpp>
#include <rtps/transport/asio_helpers.hpp>
#include <utils/SystemInfo.hpp>

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

static bool get_ipv6s(
        vector<IPFinder::info_IP>& locNames,
        bool return_loopback,
        bool force_lookup)
{
    if (!SystemInfo::get_ips(locNames, return_loopback, force_lookup))
    {
        return false;
    }
    // Controller out IP4
    auto new_end = remove_if(locNames.begin(),
                    locNames.end(),
                    [](IPFinder::info_IP ip)
                    {
                        return ip.type != IPFinder::IP6 && ip.type != IPFinder::IP6_LOCAL;
                    });
    locNames.erase(new_end, locNames.end());
    std::for_each(locNames.begin(), locNames.end(), [](IPFinder::info_IP& loc)
            {
                loc.locator.kind = LOCATOR_KIND_UDPv6;
                loc.masked_locator.kind = LOCATOR_KIND_UDPv6;
            });
    return true;
}

static bool get_ipv6s_unique_interfaces(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback,
        bool force_lookup)
{
    if (!get_ipv6s(locNames, return_loopback, force_lookup))
    {
        return false;
    }
    std::sort(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool
            {
                return a.dev < b.dev;
            });
    auto new_end = std::unique(locNames.begin(), locNames.end(),
                    [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool
                    {
                        return a.type != IPFinder::IP6_LOCAL && b.type != IPFinder::IP6_LOCAL && a.dev == b.dev;
                    });
    locNames.erase(new_end, locNames.end());
    return true;
}

static asio::ip::address_v6::bytes_type locator_to_native(
        const Locator& locator)
{
    return { { IPLocator::getIPv6(locator)[0],
        IPLocator::getIPv6(locator)[1],
        IPLocator::getIPv6(locator)[2],
        IPLocator::getIPv6(locator)[3],
        IPLocator::getIPv6(locator)[4],
        IPLocator::getIPv6(locator)[5],
        IPLocator::getIPv6(locator)[6],
        IPLocator::getIPv6(locator)[7],
        IPLocator::getIPv6(locator)[8],
        IPLocator::getIPv6(locator)[9],
        IPLocator::getIPv6(locator)[10],
        IPLocator::getIPv6(locator)[11],
        IPLocator::getIPv6(locator)[12],
        IPLocator::getIPv6(locator)[13],
        IPLocator::getIPv6(locator)[14],
        IPLocator::getIPv6(locator)[15] }
    };
}

UDPv6Transport::UDPv6Transport(
        const UDPv6TransportDescriptor& descriptor)
    : UDPTransportInterface(LOCATOR_KIND_UDPv6)
    , configuration_(descriptor)
{
    mSendBufferSize = descriptor.sendBufferSize;
    mReceiveBufferSize = descriptor.receiveBufferSize;

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
            EPROSIMA_LOG_WARNING(TRANSPORT_UDPV6,
                    "Support for interfaceWhiteList will be removed in a future release."
                    << " Please use interface allowlist/blocklist instead.");
        }

        std::vector<IPFinder::info_IP> local_interfaces;
        get_ipv6s(local_interfaces, true, false);
        for (const IPFinder::info_IP& infoIP : local_interfaces)
        {
            if (std::find_if(block_begin, block_end, [infoIP](const BlockedNetworkInterface& blocklist_element)
                    {
                        return blocklist_element.name == infoIP.dev || compare_ips(blocklist_element.name, infoIP.name);
                    }) != block_end )
            {
                // Before skipping this interface, check if present in whitelist/allowlist and warn the user if found
                if ((std::find_if(white_begin, white_end, [infoIP](const std::string& whitelist_element)
                        {
                            return whitelist_element == infoIP.dev || compare_ips(whitelist_element, infoIP.name);
                        }) != white_end ) ||
                        (std::find_if(allow_begin, allow_end,
                        [infoIP](const AllowedNetworkInterface& allowlist_element)
                        {
                            return allowlist_element.name == infoIP.dev ||
                            compare_ips(allowlist_element.name, infoIP.name);
                        }) != allow_end ))
                {
                    EPROSIMA_LOG_WARNING(TRANSPORT_UDPV6,
                            "Blocked interface " << infoIP.dev << ": " << infoIP.name
                                                 << " is also present in whitelist/allowlist."
                                                 << " Blocklist takes precedence over whitelist/allowlist.");
                }
            }
            else if (descriptor.interfaceWhiteList.empty() && descriptor.interface_allowlist.empty())
            {
                interface_whitelist_.emplace_back(ip::address_v6::from_string(infoIP.name));
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
                        return allowlist_element.name == infoIP.dev || compare_ips(allowlist_element.name,
                        infoIP.name);
                    });
                if (allow_it != allow_end)
                {
                    NetmaskFilterKind netmask_filter = allow_it->netmask_filter;
                    if (network::netmask_filter::validate_and_transform(netmask_filter,
                            descriptor.netmask_filter))
                    {
                        interface_whitelist_.emplace_back(ip::address_v6::from_string(infoIP.name));
                        allowed_interfaces_.emplace_back(infoIP.dev, infoIP.name, infoIP.masked_locator,
                                netmask_filter);
                    }
                    else
                    {
                        EPROSIMA_LOG_WARNING(TRANSPORT_UDPV6,
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
                            return whitelist_element == infoIP.dev || compare_ips(whitelist_element, infoIP.name);
                        }) != white_end )
                {
                    interface_whitelist_.emplace_back(ip::address_v6::from_string(infoIP.name));
                    allowed_interfaces_.emplace_back(infoIP.dev, infoIP.name, infoIP.masked_locator,
                            descriptor.netmask_filter);
                }
            }
        }

        if (interface_whitelist_.empty())
        {
            EPROSIMA_LOG_ERROR(TRANSPORT_UDPV6, "All whitelist interfaces were filtered out");
            interface_whitelist_.emplace_back(ip::address_v6::from_string("2001:db8::"));
        }
    }
}

UDPv6Transport::UDPv6Transport()
    : UDPTransportInterface(LOCATOR_KIND_UDPv6)
{
}

UDPv6Transport::~UDPv6Transport()
{
    clean();
}

UDPv6TransportDescriptor::UDPv6TransportDescriptor()
    : UDPTransportDescriptor()
{
}

TransportInterface* UDPv6TransportDescriptor::create_transport() const
{
    return new UDPv6Transport(*this);
}

bool UDPv6TransportDescriptor::operator ==(
        const UDPv6TransportDescriptor& t) const
{
    return (UDPTransportDescriptor::operator ==(t));
}

bool UDPv6Transport::getDefaultMetatrafficMulticastLocators(
        LocatorList& locators,
        uint32_t metatraffic_multicast_port) const
{
    Locator locator;
    locator.kind = LOCATOR_KIND_UDPv6;
    locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    IPLocator::setIPv6(locator, "ff1e::ffff:efff:1");
    locators.push_back(locator);
    return true;
}

bool UDPv6Transport::getDefaultMetatrafficUnicastLocators(
        LocatorList& locators,
        uint32_t metatraffic_unicast_port) const
{
    Locator locator;
    locator.kind = LOCATOR_KIND_UDPv6;
    locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
    locator.set_Invalid_Address();
    locators.push_back(locator);

    return true;
}

bool UDPv6Transport::getDefaultUnicastLocators(
        LocatorList& locators,
        uint32_t unicast_port) const
{
    Locator locator;
    locator.kind = LOCATOR_KIND_UDPv6;
    locator.set_Invalid_Address();
    fillUnicastLocator(locator, unicast_port);
    locators.push_back(locator);

    return true;
}

void UDPv6Transport::AddDefaultOutputLocator(
        LocatorList& defaultList)
{
    // TODO What is the default IPv6 address?
    Locator temp;
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, "ff1e::ffff:efff:1", 0, temp);
    defaultList.push_back(temp);
}

bool UDPv6Transport::compare_locator_ip(
        const Locator& lh,
        const Locator& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool UDPv6Transport::compare_locator_ip_and_port(
        const Locator& lh,
        const Locator& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void UDPv6Transport::endpoint_to_locator(
        ip::udp::endpoint& endpoint,
        Locator& locator)
{
    locator.kind = kind();
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    IPLocator::setIPv6(locator, ipBytes.data());
}

void UDPv6Transport::fill_local_ip(
        Locator& loc) const
{
    loc.kind = kind();
    IPLocator::setIPv6(loc, "::1");
}

const UDPTransportDescriptor* UDPv6Transport::configuration() const
{
    return &configuration_;
}

ip::udp::endpoint UDPv6Transport::generate_endpoint(
        const Locator& loc,
        uint16_t port)
{
    asio::ip::address_v6::bytes_type remoteAddress;
    IPLocator::copyIPv6(loc, remoteAddress.data());
    return ip::udp::endpoint(asio::ip::address_v6(remoteAddress), port);
}

asio::ip::udp::endpoint UDPv6Transport::GenerateAnyAddressEndpoint(
        uint16_t port)
{
    return ip::udp::endpoint(ip::address_v6::any(), port);
}

ip::udp::endpoint UDPv6Transport::generate_endpoint(
        const std::string& sIp,
        uint16_t port)
{
    return asio::ip::udp::endpoint(ip::address_v6::from_string(sIp), port);
}

ip::udp::endpoint UDPv6Transport::generate_endpoint(
        uint16_t port)
{
    return asio::ip::udp::endpoint(asio::ip::udp::v6(), port);
}

ip::udp::endpoint UDPv6Transport::generate_local_endpoint(
        const Locator& loc,
        uint16_t port)
{
    return ip::udp::endpoint(asio::ip::address_v6(locator_to_native(loc)), port);
}

asio::ip::udp UDPv6Transport::generate_protocol() const
{
    return ip::udp::v6();
}

bool UDPv6Transport::get_ips(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback,
        bool force_lookup) const
{
    return get_ipv6s(locNames, return_loopback, force_lookup);
}

const std::string& UDPv6Transport::localhost_name()
{
    static const std::string ip6_localhost = "::1";
    return ip6_localhost;
}

eProsimaUDPSocket UDPv6Transport::OpenAndBindInputSocket(
        const std::string& sIp,
        uint16_t port,
        bool is_multicast)
{
    eProsimaUDPSocket socket = createUDPSocket(io_service_);
    getSocketPtr(socket)->open(generate_protocol());
    if (mReceiveBufferSize != 0)
    {
        uint32_t configured_value = 0;
        uint32_t minimum_value = configuration()->maxMessageSize;
        if (!asio_helpers::asio_helpers::try_setting_buffer_size<asio::socket_base::receive_buffer_size>(
                    socket, mReceiveBufferSize, minimum_value, configured_value))
        {
            EPROSIMA_LOG_ERROR(TRANSPORT_UDPV6,
                    "Couldn't set receive buffer size to minimum value: " << minimum_value);
        }
        else if (mReceiveBufferSize != configured_value)
        {
            EPROSIMA_LOG_WARNING(TRANSPORT_UDPV6,
                    "Receive buffer size could not be set to the desired value. "
                    << "Using " << configured_value << " instead of " << mReceiveBufferSize);
        }
    }

    if (is_multicast)
    {
        getSocketPtr(socket)->set_option(ip::udp::socket::reuse_address(true));
#if defined(__QNX__)
        getSocketPtr(socket)->set_option(asio::detail::socket_option::boolean<
                    ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT>(true));
#endif // if defined(__QNX__)
    }
    else
    {
#if defined(_WIN32)
        getSocketPtr(socket)->set_option(asio::detail::socket_option::integer<
                    ASIO_OS_DEF(SOL_SOCKET), SO_EXCLUSIVEADDRUSE>(1));
#endif // if defined(_WIN32)
    }

    getSocketPtr(socket)->bind(generate_endpoint(sIp, port));

    return socket;
}

bool UDPv6Transport::OpenInputChannel(
        const Locator& locator,
        TransportReceiverInterface* receiver,
        uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!is_locator_allowed(locator))
    {
        return false;
    }

    bool success = false;

    if (!IsInputChannelOpen(locator))
    {
        success = OpenAndBindInputSockets(locator, receiver, IPLocator::isMulticast(locator), maxMsgSize);
    }

    if (IPLocator::isMulticast(locator) && IsInputChannelOpen(locator))
    {
        std::string locatorAddressStr = IPLocator::toIPv6string(locator);
        ip::address_v6 locatorAddress = ip::address_v6::from_string(locatorAddressStr);

#ifndef _WIN32
        if (!is_interface_whitelist_empty())
        {
            //Either wildcard address or the multicast address needs to be bound on non-windows systems
            bool found = false;

            // First check if the multicast address is already bound
            auto& channelResources = mInputSockets.at(IPLocator::getPhysicalPort(locator));
            for (UDPChannelResource* channelResource : channelResources)
            {
                if (locatorAddressStr == channelResource->iface())
                {
                    found = true;
                    break;
                }
            }

            // Create a new resource if no one is found
            if (!found)
            {
                try
                {
                    // Bind to multicast address
                    UDPChannelResource* p_channel_resource;
                    p_channel_resource = CreateInputChannelResource(locatorAddressStr, locator, true, maxMsgSize,
                                    receiver);
                    mInputSockets[IPLocator::getPhysicalPort(locator)].push_back(p_channel_resource);

                    // Join group on all whitelisted interfaces
                    for (auto& ip : interface_whitelist_)
                    {
                        p_channel_resource->socket()->set_option(ip::multicast::join_group(locatorAddress,
                                ip.scope_id()));
                    }
                }
                catch (asio::system_error const& e)
                {
                    EPROSIMA_LOG_WARNING(TRANSPORT_UDPV6, "UDPTransport Error binding " << locatorAddressStr << " at port: (" <<
                            IPLocator::getPhysicalPort(
                                locator) << ") with msg: " << e.what());
                    (void)e;
                }
            }
        }
        else
#endif // _WIN32
        {
            // The multicast group will be joined silently, because we do not
            // want to return another resource.
            auto pChannelResources = mInputSockets.at(IPLocator::getPhysicalPort(locator));
            for (auto& channelResource : pChannelResources)
            {
                if (channelResource->iface() == s_IPv6AddressAny)
                {
                    std::vector<IPFinder::info_IP> locNames;
                    get_ipv6s_unique_interfaces(locNames, true, false);
                    for (const auto& infoIP : locNames)
                    {
                        auto ip = asio::ip::address_v6::from_string(infoIP.name);
                        try
                        {
                            channelResource->socket()->set_option(ip::multicast::join_group(locatorAddress,
                                    ip.scope_id()));
                        }
                        catch (std::system_error& ex)
                        {
                            (void)ex;
                            EPROSIMA_LOG_WARNING(TRANSPORT_UDPV6,
                                    "Error joining multicast group on " << ip << ": " << ex.what());
                        }
                    }
                }
                else
                {
                    auto ip = asio::ip::address_v6::from_string(channelResource->iface());
                    try
                    {
                        channelResource->socket()->set_option(ip::multicast::join_group(locatorAddress, ip.scope_id()));
                    }
                    catch (std::system_error& ex)
                    {
                        (void)ex;
                        EPROSIMA_LOG_WARNING(TRANSPORT_UDPV6,
                                "Error joining multicast group on " << ip << ": " << ex.what());
                    }
                }
            }
        }
    }
    return success;
}

std::vector<std::string> UDPv6Transport::get_binding_interfaces_list()
{
    std::vector<std::string> vOutputInterfaces;
    if (is_interface_whitelist_empty())
    {
        vOutputInterfaces.push_back(s_IPv6AddressAny);
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

bool UDPv6Transport::is_interface_allowed(
        const std::string& iface) const
{
    if (interface_whitelist_.empty())
    {
        return true;
    }

    if (asio::ip::address_v6::from_string(iface) == ip::address_v6::any())
    {
        return true;
    }

    for (auto& whitelist : interface_whitelist_)
    {
        if (compare_ips(whitelist.to_string(), iface))
        {
            return true;
        }
    }

    return false;
}

bool UDPv6Transport::is_interface_whitelist_empty() const
{
    return interface_whitelist_.empty();
}

bool UDPv6Transport::is_locator_allowed(
        const Locator& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }
    if (interface_whitelist_.empty() || IPLocator::isMulticast(locator))
    {
        return true;
    }
    return is_interface_allowed(IPLocator::toIPv6string(locator));
}

bool UDPv6Transport::is_locator_reachable(
        const Locator_t& locator)
{
    return IsLocatorSupported(locator);
}

LocatorList UDPv6Transport::NormalizeLocator(
        const Locator& locator)
{
    LocatorList list;
    if (IPLocator::isAny(locator))
    {
        std::vector<IPFinder::info_IP> locNames;
        get_ipv6s(locNames, false, false);
        for (const auto& infoIP : locNames)
        {
            if (is_interface_allowed(infoIP.name))
            {
                Locator newloc(locator);
                IPLocator::setIPv6(newloc, infoIP.locator);
                list.push_back(newloc);
            }
        }

        if (list.empty())
        {
            Locator newloc(locator);
            IPLocator::setIPv6(newloc, "::1");
            list.push_back(newloc);
        }
    }
    else
    {
        list.push_back(locator);
    }

    return list;
}

bool UDPv6Transport::is_local_locator(
        const Locator& locator) const
{
    assert(locator.kind == LOCATOR_KIND_UDPv6);

    if (IPLocator::isLocal(locator))
    {
        return true;
    }

    std::vector<IPFinder::info_IP> currentInterfaces;
    if (!get_ips(currentInterfaces, false, false))
    {
        EPROSIMA_LOG_WARNING(TRANSPORT_UDPV6,
                "Could not retrieve IPs information to check if locator " << locator << " is local.");
        return false;
    }
    for (const IPFinder::info_IP& localInterface : currentInterfaces)
    {
        if (IPLocator::compareAddress(localInterface.locator, locator))
        {
            return true;
        }
    }

    return false;
}

void UDPv6Transport::set_receive_buffer_size(
        uint32_t size)
{
    configuration_.receiveBufferSize = size;
}

void UDPv6Transport::set_send_buffer_size(
        uint32_t size)
{
    configuration_.sendBufferSize = size;
}

void UDPv6Transport::SetSocketOutboundInterface(
        eProsimaUDPSocket& socket,
        const std::string& sIp)
{
#ifdef __APPLE__
    Locator loc;
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, sIp, 0, loc);
    if (IPLocator::isLocal(loc))
    {
        return;
    }
#endif // ifdef __APPLE__
    getSocketPtr(socket)->set_option(ip::multicast::outbound_interface(
                asio::ip::address_v6::from_string(sIp).scope_id()));
}

bool UDPv6Transport::compare_ips(
        const std::string& ip1,
        const std::string& ip2)
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

void UDPv6Transport::update_network_interfaces()
{
    UDPTransportInterface::update_network_interfaces();
    // TODO(jlbueno)
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
