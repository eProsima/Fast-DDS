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
#include <fastdds/rtps/messages/MessageReceiver.h>
#include <fastdds/rtps/transport/SenderResource.h>
#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastrtps/utils/IPLocator.h>

using namespace std;
using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using IPFinder = fastrtps::rtps::IPFinder;
using IPLocator = fastrtps::rtps::IPLocator;

// TODO: move to SocketTransportInterface? not straightforward as overloaded in TCPv4Transport
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
    : UDPTransportInterface(LOCATOR_KIND_UDPv6, descriptor)
    , configuration_(descriptor)
{
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

void UDPv6Transport::endpoint_to_locator(
        ip::udp::endpoint& endpoint,
        Locator& locator)
{
    locator.kind = kind();
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    IPLocator::setIPv6(locator, ipBytes.data());
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
        if (!is_interface_allowlist_empty())
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

                    // Join group on all allowed interfaces
                    for (auto& iface : allowed_interfaces_)
                    {
                        p_channel_resource->socket()->set_option(ip::multicast::join_group(locatorAddress,
                                asio::ip::address_v6::from_string(iface.ip).scope_id()));
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
                    get_ips_unique_interfaces(locNames, true, false);
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

void UDPv6Transport::update_network_interfaces()
{
    UDPTransportInterface::update_network_interfaces();
    // TODO(jlbueno)
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
