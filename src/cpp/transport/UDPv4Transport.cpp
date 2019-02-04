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

#include <fastrtps/transport/TransportInterface.h>
#include <fastrtps/transport/UDPv4Transport.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/network/SenderResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps{

static void GetIP4s(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    auto new_end = remove_if(locNames.begin(),
            locNames.end(),
            [](IPFinder::info_IP ip){return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL;});
    locNames.erase(new_end, locNames.end());
    std::for_each(locNames.begin(), locNames.end(), [](IPFinder::info_IP& loc)
    {
        loc.locator.kind = LOCATOR_KIND_UDPv4;
    });
}

static void GetIP4sUniqueInterfaces(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    GetIP4s(locNames, return_loopback);
    std::sort(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool {return a.dev < b.dev;});
    auto new_end = std::unique(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool {return a.type != IPFinder::IP4_LOCAL && b.type != IPFinder::IP4_LOCAL && a.dev == b.dev;});
    locNames.erase(new_end, locNames.end());
}

static asio::ip::address_v4::bytes_type locatorToNative(const Locator_t& locator)
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

UDPv4Transport::UDPv4Transport(const UDPv4TransportDescriptor& descriptor)
    : mConfiguration_(descriptor)
{
    mTransportKind = LOCATOR_KIND_UDPv4;
    mSendBufferSize = descriptor.sendBufferSize;
    mReceiveBufferSize = descriptor.receiveBufferSize;
    for (const auto& interface : descriptor.interfaceWhiteList)
        mInterfaceWhiteList.emplace_back(ip::address_v4::from_string(interface));
}

UDPv4TransportDescriptor::UDPv4TransportDescriptor()
    : UDPTransportDescriptor()
{
}

UDPv4TransportDescriptor::UDPv4TransportDescriptor(const UDPv4TransportDescriptor& t)
    : UDPTransportDescriptor(t)
{
}

TransportInterface* UDPv4TransportDescriptor::create_transport() const
{
    return new UDPv4Transport(*this);
}

UDPv4Transport::UDPv4Transport()
{
    mTransportKind = LOCATOR_KIND_UDPv4;
}

UDPv4Transport::~UDPv4Transport()
{
    Clean();
}

bool UDPv4Transport::getDefaultMetatrafficMulticastLocators(LocatorList_t &locators,
    uint32_t metatraffic_multicast_port) const
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_UDPv4;
    locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locators.push_back(locator);
    return true;
}

bool UDPv4Transport::getDefaultMetatrafficUnicastLocators(LocatorList_t &locators,
    uint32_t metatraffic_unicast_port) const
{
    if (mInterfaceWhiteList.empty())
    {
        Locator_t locator;
        locator.kind = LOCATOR_KIND_UDPv4;
        locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
        locator.set_Invalid_Address();
        locators.push_back(locator);
    }
    else
    {
        for (auto& it : mInterfaceWhiteList)
        {
            Locator_t locator;
            locator.kind = LOCATOR_KIND_UDPv4;
            locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
            IPLocator::setIPv4(locator, it.to_string());
            locators.push_back(locator);
        }
    }

    return true;
}

bool UDPv4Transport::getDefaultUnicastLocators(LocatorList_t &locators, uint32_t unicast_port) const
{
    if (mInterfaceWhiteList.empty())
    {
        Locator_t locator;
        locator.kind = LOCATOR_KIND_UDPv4;
        locator.set_Invalid_Address();
        fillUnicastLocator(locator, unicast_port);
        locators.push_back(locator);
    }
    else
    {
        for (auto& it : mInterfaceWhiteList)
        {
            Locator_t locator;
            locator.kind = LOCATOR_KIND_UDPv4;
            IPLocator::setIPv4(locator, it.to_string());
            fillUnicastLocator(locator, unicast_port);
            locators.push_back(locator);
        }
    }
    return true;
}

void UDPv4Transport::AddDefaultOutputLocator(LocatorList_t &defaultList)
{
    Locator_t locator;
    IPLocator::createLocator(LOCATOR_KIND_UDPv4, "239.255.0.1", mConfiguration_.m_output_udp_socket, locator);
    defaultList.push_back(locator);
}

bool UDPv4Transport::CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool UDPv4Transport::CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void UDPv4Transport::EndpointToLocator(ip::udp::endpoint& endpoint, Locator_t& locator)
{
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    IPLocator::setIPv4(locator, ipBytes.data());
}

void UDPv4Transport::FillLocalIp(Locator_t& loc)
{
    IPLocator::setIPv4(loc, "127.0.0.1");
    loc.kind = LOCATOR_KIND_UDPv4;
}

const UDPTransportDescriptor* UDPv4Transport::GetConfiguration() const
{
    return &mConfiguration_;
}

asio::ip::udp::endpoint UDPv4Transport::GenerateAnyAddressEndpoint(uint16_t port)
{
    return ip::udp::endpoint(ip::address_v4::any(), port);
}

ip::udp::endpoint UDPv4Transport::GenerateEndpoint(const Locator_t& loc, uint16_t port)
{
    asio::ip::address_v4::bytes_type remoteAddress;
    IPLocator::copyIPv4(loc, remoteAddress.data());
    return ip::udp::endpoint(asio::ip::address_v4(remoteAddress), port);
}

ip::udp::endpoint UDPv4Transport::GenerateEndpoint(const std::string& sIp, uint16_t port)
{
    return asio::ip::udp::endpoint(ip::address_v4::from_string(sIp), port);
}

ip::udp::endpoint UDPv4Transport::GenerateEndpoint(uint16_t port)
{
    return asio::ip::udp::endpoint(asio::ip::udp::v4(), port);
}

ip::udp::endpoint UDPv4Transport::GenerateLocalEndpoint(const Locator_t& loc, uint16_t port)
{
    return ip::udp::endpoint(asio::ip::address_v4(locatorToNative(loc)), port);
}

asio::ip::udp UDPv4Transport::GenerateProtocol() const
{
    return ip::udp::v4();
}

void UDPv4Transport::GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback)
{
    GetIP4s(locNames, return_loopback);
}

eProsimaUDPSocket UDPv4Transport::OpenAndBindInputSocket(const std::string& sIp, uint16_t port, bool is_multicast)
{
    eProsimaUDPSocket socket = createUDPSocket(mService);
    getSocketPtr(socket)->open(GenerateProtocol());
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
#endif
    }

    getSocketPtr(socket)->bind(GenerateEndpoint(sIp, port));
    return socket;
}

bool UDPv4Transport::OpenInputChannel(const Locator_t& locator, TransportReceiverInterface* receiver,
    uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    bool success = false;

    if (!IsInputChannelOpen(locator))
        success = OpenAndBindInputSockets(locator, receiver, IPLocator::isMulticast(locator), maxMsgSize);

    if (IPLocator::isMulticast(locator) && IsInputChannelOpen(locator))
    {
        std::string locatorAddressStr = IPLocator::toIPv4string(locator);
        ip::address_v4 locatorAddress = ip::address_v4::from_string(locatorAddressStr);

#ifndef _WIN32
        if (!IsInterfaceWhiteListEmpty())
        {
            // Either wildcard address or the multicast address needs to be bound on non-windows systems
            bool found = false;

            // First check if the multicast address is already bound
            auto& channelResources = mInputSockets.at(IPLocator::getPhysicalPort(locator));
            for (UDPChannelResource* channelResource : channelResources)
            {
                if (channelResource->GetInterface() == locatorAddressStr)
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
                    UDPChannelResource* pChannelResource;
                    pChannelResource = CreateInputChannelResource(locatorAddressStr, locator, true, maxMsgSize, receiver);
                    mInputSockets[IPLocator::getPhysicalPort(locator)].push_back(pChannelResource);

                    // Join group on all whitelisted interfaces
                    for (auto& ip : mInterfaceWhiteList)
                    {
                        pChannelResource->getSocket()->set_option(ip::multicast::join_group(locatorAddress, ip));
                    }
                }
                catch (asio::system_error const& e)
                {
                    (void)e;
                    logWarning(RTPS_MSG_OUT, "UDPTransport Error binding " << locatorAddressStr << " at port: (" << IPLocator::getPhysicalPort(locator) << ")"
                        << " with msg: " << e.what());
                }
            }
        }
        else
#endif
        {
            // The multicast group will be joined silently, because we do not
            // want to return another resource.
            auto& channelResources = mInputSockets.at(IPLocator::getPhysicalPort(locator));
            for (UDPChannelResource* channelResource : channelResources)
            {
                if (channelResource->GetInterface() == s_IPv4AddressAny)
                {
                    std::vector<IPFinder::info_IP> locNames;
                    GetIP4sUniqueInterfaces(locNames, true);
                    for (const auto& infoIP : locNames)
                    {
                        auto ip = asio::ip::address_v4::from_string(infoIP.name);
                        try
                        {
                            channelResource->getSocket()->set_option(ip::multicast::join_group(locatorAddress, ip));
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
                    auto ip = asio::ip::address_v4::from_string(channelResource->GetInterface());
                    try
                    {
                        channelResource->getSocket()->set_option(ip::multicast::join_group(locatorAddress, ip));
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

std::vector<std::string> UDPv4Transport::GetBindingInterfacesList()
{
    std::vector<std::string> vOutputInterfaces;
    if (IsInterfaceWhiteListEmpty())
    {
        vOutputInterfaces.push_back(s_IPv4AddressAny);
    }
    else
    {
        for (auto& ip : mInterfaceWhiteList)
        {
            vOutputInterfaces.push_back(ip.to_string());
        }
    }

    return vOutputInterfaces;
}

bool UDPv4Transport::IsInterfaceAllowed(const std::string& interface) const
{
    return IsInterfaceAllowed(asio::ip::address_v4::from_string(interface));
}

bool UDPv4Transport::IsInterfaceAllowed(const ip::address_v4& ip) const
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v4::any())
        return true;

    return find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}

bool UDPv4Transport::IsInterfaceWhiteListEmpty() const
{
    return mInterfaceWhiteList.empty();
}

bool UDPv4Transport::IsLocatorAllowed(const Locator_t& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }
    if (mInterfaceWhiteList.empty() || IPLocator::isMulticast(locator))
    {
        return true;
    }
    return IsInterfaceAllowed(IPLocator::toIPv4string(locator));
}

LocatorList_t UDPv4Transport::NormalizeLocator(const Locator_t& locator)
{
    LocatorList_t list;

    if (IPLocator::isAny(locator))
    {
        std::vector<IPFinder::info_IP> locNames;
        GetIP4s(locNames);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v4::from_string(infoIP.name);
            if (IsInterfaceAllowed(ip))
            {
                Locator_t newloc(locator);
                IPLocator::setIPv4(newloc, infoIP.locator);
                list.push_back(newloc);
            }
        }
        if (list.empty())
        {
            Locator_t newloc(locator);
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

bool UDPv4Transport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_UDPv4);

    if(IPLocator::isLocal(locator))
        return true;

    for(auto localInterface : currentInterfaces)
        if(IPLocator::compareAddress(locator, localInterface.locator))
        {
            return true;
        }

    return false;
}

void UDPv4Transport::SetReceiveBufferSize(uint32_t size)
{
    mConfiguration_.receiveBufferSize = size;
}

void UDPv4Transport::SetSendBufferSize(uint32_t size)
{
    mConfiguration_.sendBufferSize = size;
}

void UDPv4Transport::SetSocketOutboundInterface(eProsimaUDPSocket& socket, const std::string& sIp)
{
	getSocketPtr(socket)->set_option(ip::multicast::outbound_interface(asio::ip::address_v4::from_string(sIp)));
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
