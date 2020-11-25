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

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/transport/UDPv6Transport.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastdds/rtps/network/SenderResource.h>
#include <fastdds/rtps/messages/MessageReceiver.h>

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using Locator_t = fastrtps::rtps::Locator_t;
using IPFinder = fastrtps::rtps::IPFinder;
using octet = fastrtps::rtps::octet;
using IPLocator = fastrtps::rtps::IPLocator;
using LocatorList_t = fastrtps::rtps::LocatorList_t;
using Log = fastdds::dds::Log;

static void get_ipv6s(
        vector<IPFinder::info_IP>& locNames,
        bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
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
            });
}

static void get_ipv6s_unique_interfaces(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback = false)
{
    get_ipv6s(locNames, return_loopback);
    std::sort(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool
            {
                return a.dev < b.dev;
            });
    auto new_end = std::unique(locNames.begin(), locNames.end(),
                    [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool
                    {
                        return a.dev == b.dev;
                    });
    locNames.erase(new_end, locNames.end());
}

static asio::ip::address_v6::bytes_type locator_to_native(
        const Locator_t& locator)
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
        IPLocator::getIPv6(locator)[15] } };
}

UDPv6Transport::UDPv6Transport(
        const UDPv6TransportDescriptor& descriptor)
    : UDPTransportInterface(LOCATOR_KIND_UDPv6)
    , configuration_(descriptor)
{
    mSendBufferSize = descriptor.sendBufferSize;
    mReceiveBufferSize = descriptor.receiveBufferSize;
    for (const auto& interface : descriptor.interfaceWhiteList)
    {
        interface_whitelist_.emplace_back(ip::address_v6::from_string(interface));
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

UDPv6TransportDescriptor::UDPv6TransportDescriptor(
        const UDPv6TransportDescriptor& t)
    : UDPTransportDescriptor(t)
{
}

TransportInterface* UDPv6TransportDescriptor::create_transport() const
{
    return new UDPv6Transport(*this);
}

bool UDPv6Transport::getDefaultMetatrafficMulticastLocators(
        LocatorList_t& locators,
        uint32_t metatraffic_multicast_port) const
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv6;
    locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    IPLocator::setIPv6(locator, "ff31::8000:1234");
    locators.push_back(locator);
    return true;
}

bool UDPv6Transport::getDefaultMetatrafficUnicastLocators(
        LocatorList_t& locators,
        uint32_t metatraffic_unicast_port) const
{
    if (interface_whitelist_.empty())
    {
        Locator_t locator;
        locator.kind = LOCATOR_KIND_UDPv6;
        locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
        locator.set_Invalid_Address();
        locators.push_back(locator);
    }
    else
    {
        for (auto& it : interface_whitelist_)
        {
            Locator_t locator;
            locator.kind = LOCATOR_KIND_UDPv6;
            locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
            IPLocator::setIPv6(locator, it.to_string());
            locators.push_back(locator);
        }
    }
    return true;
}

bool UDPv6Transport::getDefaultUnicastLocators(
        LocatorList_t& locators,
        uint32_t unicast_port) const
{
    if (interface_whitelist_.empty())
    {
        Locator_t locator;
        locator.kind = LOCATOR_KIND_UDPv6;
        locator.set_Invalid_Address();
        fillUnicastLocator(locator, unicast_port);
        locators.push_back(locator);
    }
    else
    {
        for (auto& it : interface_whitelist_)
        {
            Locator_t locator;
            locator.kind = LOCATOR_KIND_UDPv6;
            IPLocator::setIPv6(locator, it.to_string());
            fillUnicastLocator(locator, unicast_port);
            locators.push_back(locator);
        }
    }
    return true;
}

void UDPv6Transport::AddDefaultOutputLocator(
        LocatorList_t& defaultList)
{
    // TODO What is the default IPv6 address?
    Locator_t temp;
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, "239.255.0.1", 0, temp);
    defaultList.push_back(temp);
}

bool UDPv6Transport::compare_locator_ip(
        const Locator_t& lh,
        const Locator_t& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool UDPv6Transport::compare_locator_ip_and_port(
        const Locator_t& lh,
        const Locator_t& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void UDPv6Transport::endpoint_to_locator(
        ip::udp::endpoint& endpoint,
        Locator_t& locator)
{
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    IPLocator::setIPv6(locator, ipBytes.data());
}

void UDPv6Transport::fill_local_ip(
        Locator_t& loc) const
{
    IPLocator::setIPv6(loc, "::1");
    loc.kind = LOCATOR_KIND_UDPv6;
}

const UDPTransportDescriptor* UDPv6Transport::configuration() const
{
    return &configuration_;
}

ip::udp::endpoint UDPv6Transport::generate_endpoint(
        const Locator_t& loc,
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
        const Locator_t& loc,
        uint16_t port)
{
    return ip::udp::endpoint(asio::ip::address_v6(locator_to_native(loc)), port);
}

asio::ip::udp UDPv6Transport::generate_protocol() const
{
    return ip::udp::v6();
}

void UDPv6Transport::get_ips(
        std::vector<IPFinder::info_IP>& locNames,
        bool return_loopback)
{
    get_ipv6s(locNames, return_loopback);
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

bool UDPv6Transport::OpenInputChannel(
        const Locator_t& locator,
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
        // The multicast group will be joined silently, because we do not
        // want to return another resource.
        auto pChannelResources = mInputSockets.at(IPLocator::getPhysicalPort(locator));
        for (auto& channelResource : pChannelResources)
        {
            if (channelResource->interface() == s_IPv4AddressAny)
            {
                std::vector<IPFinder::info_IP> locNames;
                get_ipv6s_unique_interfaces(locNames, true);
                for (const auto& infoIP : locNames)
                {
                    auto ip = asio::ip::address_v6::from_string(infoIP.name);
                    try
                    {
                        channelResource->socket()->set_option(ip::multicast::join_group(
                                    ip::address_v6::from_string(IPLocator::toIPv6string(locator)), ip.scope_id()));
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
                auto ip = asio::ip::address_v6::from_string(channelResource->interface());
                try
                {
                    channelResource->socket()->set_option(ip::multicast::join_group(
                                ip::address_v6::from_string(IPLocator::toIPv6string(locator)), ip.scope_id()));
                }
                catch (std::system_error& ex)
                {
                    (void)ex;
                    logWarning(RTPS_MSG_OUT, "Error joining multicast group on " << ip << ": " << ex.what());
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
        const std::string& interface) const
{
    return is_interface_allowed(asio::ip::address_v6::from_string(interface));
}

bool UDPv6Transport::is_interface_allowed(
        const ip::address_v6& ip) const
{
    if (interface_whitelist_.empty())
    {
        return true;
    }

    if (ip == ip::address_v6::any())
    {
        return true;
    }

    return find(interface_whitelist_.begin(), interface_whitelist_.end(), ip) != interface_whitelist_.end();
}

bool UDPv6Transport::is_interface_whitelist_empty() const
{
    return interface_whitelist_.empty();
}

bool UDPv6Transport::is_locator_allowed(
        const Locator_t& locator) const
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

LocatorList_t UDPv6Transport::NormalizeLocator(
        const Locator_t& locator)
{
    LocatorList_t list;
    if (IPLocator::isAny(locator))
    {
        std::vector<IPFinder::info_IP> locNames;
        get_ipv6s(locNames);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v6::from_string(infoIP.name);
            if (is_interface_allowed(ip))
            {
                Locator_t newloc(locator);
                IPLocator::setIPv6(newloc, infoIP.locator);
                list.push_back(newloc);
            }
        }

        if (list.empty())
        {
            Locator_t newloc(locator);
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
        const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_UDPv6);

    if (IPLocator::isLocal(locator))
    {
        return true;
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
    getSocketPtr(socket)->set_option(ip::multicast::outbound_interface(
                asio::ip::address_v6::from_string(sIp).scope_id()));
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
