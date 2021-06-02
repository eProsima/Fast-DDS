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

#include <rtps/transport/UDPv4Transport.h>

#include <utility>
#include <cstring>
#include <algorithm>

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/network/ReceiverResource.h>
#include <fastdds/rtps/network/SenderResource.h>
#include <fastdds/rtps/messages/CDRMessage.h>
#include <fastdds/rtps/messages/MessageReceiver.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/IPLocator.h>

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using IPFinder = fastrtps::rtps::IPFinder;
using IPLocator = fastrtps::rtps::IPLocator;
using Log = fastdds::dds::Log;

static void get_ipv4s(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    auto new_end = remove_if(locNames.begin(),
                    locNames.end(),
                    [](IPFinder::info_IP ip)
                    {
                        return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL;
                    });
    locNames.erase(new_end, locNames.end());
    std::for_each(locNames.begin(), locNames.end(), [](IPFinder::info_IP& loc)
            {
                loc.locator.kind = LOCATOR_KIND_UDPv4;
            });
}

static void get_ipv4s_unique_interfaces(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback = false)
{
    get_ipv4s(locNames, return_loopback);
    std::sort(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool
            {
                return a.dev < b.dev;
            });
    auto new_end = std::unique(locNames.begin(), locNames.end(),
                    [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool
                    {
                        return a.type != IPFinder::IP4_LOCAL && b.type != IPFinder::IP4_LOCAL && a.dev == b.dev;
                    });
    locNames.erase(new_end, locNames.end());
}

static asio::ip::address_v4::bytes_type locator_to_native(
        const Locator& locator)
{
    if (IPLocator::hasWan(locator))
    {
        return{ { IPLocator::getWan(locator)[0],
            IPLocator::getWan(locator)[1],
            IPLocator::getWan(locator)[2],
            IPLocator::getWan(locator)[3] } };
    }
    else
    {
        return{ { IPLocator::getIPv4(locator)[0],
            IPLocator::getIPv4(locator)[1],
            IPLocator::getIPv4(locator)[2],
            IPLocator::getIPv4(locator)[3] } };
    }
}

UDPv4Transport::UDPv4Transport(
        const UDPv4TransportDescriptor& descriptor)
    : UDPTransportInterface(LOCATOR_KIND_UDPv4)
    , configuration_(descriptor)
{
    mSendBufferSize = descriptor.sendBufferSize;
    mReceiveBufferSize = descriptor.receiveBufferSize;
    if (!descriptor.interfaceWhiteList.empty())
    {
        const auto white_begin = descriptor.interfaceWhiteList.begin();
        const auto white_end = descriptor.interfaceWhiteList.end();

        std::vector<IPFinder::info_IP> local_interfaces;
        get_ipv4s(local_interfaces, true);
        for (const IPFinder::info_IP& infoIP : local_interfaces)
        {
            if (std::find(white_begin, white_end, infoIP.name) != white_end)
            {
                interface_whitelist_.emplace_back(ip::address_v4::from_string(infoIP.name));
            }
        }

        if (interface_whitelist_.empty())
        {
            logError(TRANSPORT, "All whitelist interfaces were filtered out");
            interface_whitelist_.emplace_back(ip::address_v4::from_string("192.0.2.0"));
        }
    }
}

UDPv4Transport::UDPv4Transport()
    : UDPTransportInterface(LOCATOR_KIND_UDPv4)
{
}

UDPv4Transport::~UDPv4Transport()
{
    clean();
}

UDPv4TransportDescriptor::UDPv4TransportDescriptor()
    : UDPTransportDescriptor()
{
}

TransportInterface* UDPv4TransportDescriptor::create_transport() const
{
    return new UDPv4Transport(*this);
}

bool UDPv4TransportDescriptor::operator ==(
        const UDPv4TransportDescriptor& t) const
{
    return (UDPTransportDescriptor::operator ==(t));
}

bool UDPv4Transport::getDefaultMetatrafficMulticastLocators(
        LocatorList& locators,
        uint32_t metatraffic_multicast_port) const
{
    Locator locator;
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locators.push_back(locator);
    return true;
}

bool UDPv4Transport::getDefaultMetatrafficUnicastLocators(
        LocatorList& locators,
        uint32_t metatraffic_unicast_port) const
{
    Locator locator;
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
    locator.set_Invalid_Address();
    locators.push_back(locator);

    return true;
}

bool UDPv4Transport::getDefaultUnicastLocators(
        LocatorList& locators,
        uint32_t unicast_port) const
{
    Locator locator;
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.set_Invalid_Address();
    fillUnicastLocator(locator, unicast_port);
    locators.push_back(locator);

    return true;
}

void UDPv4Transport::AddDefaultOutputLocator(
        LocatorList& defaultList)
{
    Locator locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "239.255.0.1", configuration_.m_output_udp_socket, locator);
    defaultList.push_back(locator);
}

bool UDPv4Transport::compare_locator_ip(
        const Locator& lh,
        const Locator& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool UDPv4Transport::compare_locator_ip_and_port(
        const Locator& lh,
        const Locator& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void UDPv4Transport::endpoint_to_locator(
        ip::udp::endpoint& endpoint,
        Locator& locator)
{
    locator.kind = kind();
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    IPLocator::setIPv4(locator, ipBytes.data());
}

void UDPv4Transport::fill_local_ip(
        Locator& loc) const
{
    IPLocator::setIPv4(loc, "127.0.0.1");
    loc.kind = LOCATOR_KIND_UDPv4;
}

const UDPTransportDescriptor* UDPv4Transport::configuration() const
{
    return &configuration_;
}

asio::ip::udp::endpoint UDPv4Transport::GenerateAnyAddressEndpoint(
        uint16_t port)
{
    return ip::udp::endpoint(ip::address_v4::any(), port);
}

ip::udp::endpoint UDPv4Transport::generate_endpoint(
        const Locator& loc,
        uint16_t port)
{
    asio::ip::address_v4::bytes_type remoteAddress;
    IPLocator::copyIPv4(loc, remoteAddress.data());
    return ip::udp::endpoint(asio::ip::address_v4(remoteAddress), port);
}

ip::udp::endpoint UDPv4Transport::generate_endpoint(
        const std::string& sIp,
        uint16_t port)
{
    return asio::ip::udp::endpoint(ip::address_v4::from_string(sIp), port);
}

ip::udp::endpoint UDPv4Transport::generate_endpoint(
        uint16_t port)
{
    return asio::ip::udp::endpoint(asio::ip::udp::v4(), port);
}

ip::udp::endpoint UDPv4Transport::generate_local_endpoint(
        const Locator& loc,
        uint16_t port)
{
    return ip::udp::endpoint(asio::ip::address_v4(locator_to_native(loc)), port);
}

asio::ip::udp UDPv4Transport::generate_protocol() const
{
    return ip::udp::v4();
}

void UDPv4Transport::get_ips(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback)
{
    get_ipv4s(locNames, return_loopback);
}

const std::string& UDPv4Transport::localhost_name()
{
    static const std::string ip4_localhost = "127.0.0.1";
    return ip4_localhost;
}

eProsimaUDPSocket UDPv4Transport::OpenAndBindInputSocket(
        const std::string& sIp,
        uint16_t port,
        bool is_multicast)
{
    eProsimaUDPSocket socket = createUDPSocket(io_service_);
    getSocketPtr(socket)->open(generate_protocol());
    if (mReceiveBufferSize != 0)
    {
        getSocketPtr(socket)->set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    }

    if (is_multicast)
    {
        getSocketPtr(socket)->set_option(ip::udp::socket::reuse_address(true));
#if defined(__QNX__)
        getSocketPtr(socket)->set_option(asio::detail::socket_option::boolean<
                    ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT>(true));
#endif // if defined(__QNX__)
    }

    getSocketPtr(socket)->bind(generate_endpoint(sIp, port));
    return socket;
}

bool UDPv4Transport::OpenInputChannel(
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
        std::string locatorAddressStr = IPLocator::toIPv4string(locator);
        ip::address_v4 locatorAddress = ip::address_v4::from_string(locatorAddressStr);

#ifndef _WIN32
        if (!is_interface_whitelist_empty())
        {
            // Either wildcard address or the multicast address needs to be bound on non-windows systems
            bool found = false;

            // First check if the multicast address is already bound
            auto& channelResources = mInputSockets.at(IPLocator::getPhysicalPort(locator));
            for (UDPChannelResource* channelResource : channelResources)
            {
                if (channelResource->interface() == locatorAddressStr)
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
                        p_channel_resource->socket()->set_option(ip::multicast::join_group(locatorAddress, ip));
                    }
                }
                catch (asio::system_error const& e)
                {
                    (void)e;
                    logWarning(RTPS_MSG_OUT, "UDPTransport Error binding " << locatorAddressStr << " at port: (" << IPLocator::getPhysicalPort(
                                locator) << ")"
                                                                           << " with msg: " << e.what());
                }
            }
        }
        else
#endif // ifndef _WIN32
        {
            // The multicast group will be joined silently, because we do not
            // want to return another resource.
            auto& channelResources = mInputSockets.at(IPLocator::getPhysicalPort(locator));
            for (UDPChannelResource* channelResource : channelResources)
            {
                if (channelResource->interface() == s_IPv4AddressAny)
                {
                    std::vector<IPFinder::info_IP> locNames;
                    get_ipv4s_unique_interfaces(locNames, true);
                    for (const auto& infoIP : locNames)
                    {
                        auto ip = asio::ip::address_v4::from_string(infoIP.name);
                        try
                        {
                            channelResource->socket()->set_option(ip::multicast::join_group(locatorAddress, ip));
                        }
                        catch (std::system_error& ex)
                        {
                            (void)ex;
                            logWarning(RTPS_MSG_OUT, "Error joining multicast group on " << ip << ": " << ex.what());
                        }
                    }
                }
                else
                {
                    auto ip = asio::ip::address_v4::from_string(channelResource->interface());
                    try
                    {
                        channelResource->socket()->set_option(ip::multicast::join_group(locatorAddress, ip));
                    }
                    catch (std::system_error& ex)
                    {
                        (void)ex;
                        logWarning(RTPS_MSG_OUT, "Error joining multicast group on " << ip << ": " << ex.what());
                    }
                }
            }
        }
    }

    return success;
}

std::vector<std::string> UDPv4Transport::get_binding_interfaces_list()
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

bool UDPv4Transport::is_interface_allowed(
        const std::string& interface) const
{
    return is_interface_allowed(asio::ip::address_v4::from_string(interface));
}

bool UDPv4Transport::is_interface_allowed(
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

bool UDPv4Transport::is_interface_whitelist_empty() const
{
    return interface_whitelist_.empty();
}

bool UDPv4Transport::is_locator_allowed(
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
    return is_interface_allowed(IPLocator::toIPv4string(locator));
}

LocatorList UDPv4Transport::NormalizeLocator(
        const Locator& locator)
{
    LocatorList list;

    if (IPLocator::isAny(locator))
    {
        std::vector<IPFinder::info_IP> locNames;
        get_ipv4s(locNames);
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

bool UDPv4Transport::is_local_locator(
        const Locator& locator) const
{
    assert(locator.kind == LOCATOR_KIND_UDPv4);

    if (IPLocator::isLocal(locator))
    {
        return true;
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

void UDPv4Transport::set_receive_buffer_size(
        uint32_t size)
{
    configuration_.receiveBufferSize = size;
}

void UDPv4Transport::set_send_buffer_size(
        uint32_t size)
{
    configuration_.sendBufferSize = size;
}

void UDPv4Transport::SetSocketOutboundInterface(
        eProsimaUDPSocket& socket,
        const std::string& sIp)
{
    getSocketPtr(socket)->set_option(ip::multicast::outbound_interface(asio::ip::address_v4::from_string(sIp)));
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
