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

#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/transport/TCPv6Transport.h>
#include <fastrtps/transport/tcp/RTCPMessageManager.h>
#include <fastrtps/transport/timedevent/CleanTCPSocketsEvent.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include "asio.hpp"
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/network/SenderResource.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

static void GetIP6s(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    auto new_end = remove_if(locNames.begin(),
        locNames.end(),
        [](IPFinder::info_IP ip) {return ip.type != IPFinder::IP6 && ip.type != IPFinder::IP6_LOCAL; });
    locNames.erase(new_end, locNames.end());
    std::for_each(locNames.begin(), locNames.end(), [](IPFinder::info_IP& loc)
    {
        loc.locator.kind = LOCATOR_KIND_TCPv6;
    });
}

static asio::ip::address_v6::bytes_type locatorToNative(Locator_t& locator)
{
    return{ { IPLocator::getIPv6(locator)[0],
        IPLocator::getIPv6(locator)[1],
        IPLocator::getIPv6(locator)[2],
        IPLocator::getIPv6(locator)[3] ,
        IPLocator::getIPv6(locator)[4] ,
        IPLocator::getIPv6(locator)[5],
        IPLocator::getIPv6(locator)[6] ,
        IPLocator::getIPv6(locator)[7] ,
        IPLocator::getIPv6(locator)[8],
        IPLocator::getIPv6(locator)[9] ,
        IPLocator::getIPv6(locator)[10] ,
        IPLocator::getIPv6(locator)[11],
        IPLocator::getIPv6(locator)[12] ,
        IPLocator::getIPv6(locator)[13] ,
        IPLocator::getIPv6(locator)[14],
        IPLocator::getIPv6(locator)[15] } };
}

TCPv6Transport::TCPv6Transport(const TCPv6TransportDescriptor& descriptor)
    : mConfiguration_(descriptor)
{
    mTransportKind = LOCATOR_KIND_TCPv6;
    for (const auto& interface : descriptor.interfaceWhiteList)
    {
        mInterfaceWhiteList.emplace_back(ip::address_v6::from_string(interface));
    }

    for (uint16_t port : mConfiguration_.listening_ports)
    {
        Locator_t locator(LOCATOR_KIND_TCPv6, port);
        CreateAcceptorSocket(locator);
    }
}

TCPv6TransportDescriptor::TCPv6TransportDescriptor()
    : TCPTransportDescriptor()
{
}

TCPv6TransportDescriptor::TCPv6TransportDescriptor(const TCPv6TransportDescriptor& t)
    : TCPTransportDescriptor(t)
{
}

TransportInterface* TCPv6TransportDescriptor::create_transport() const
{
    return new TCPv6Transport(*this);
}

TCPv6Transport::TCPv6Transport()
{
    mTransportKind = LOCATOR_KIND_TCPv6;
}

TCPv6Transport::~TCPv6Transport()
{
    Clean();
}

void TCPv6Transport::AddDefaultOutputLocator(LocatorList_t& /*defaultList*/)
{
}

const TCPTransportDescriptor* TCPv6Transport::GetConfiguration() const
{
    return &mConfiguration_;
}

TCPTransportDescriptor* TCPv6Transport::GetConfiguration()
{
    return &mConfiguration_;
}

void TCPv6Transport::GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback) const
{
    GetIP6s(locNames, return_loopback);
}

uint16_t TCPv6Transport::GetLogicalPortRange() const
{
    return mConfiguration_.logical_port_range;
}

uint16_t TCPv6Transport::GetLogicalPortIncrement() const
{
    return mConfiguration_.logical_port_increment;
}

uint16_t TCPv6Transport::GetMaxLogicalPort() const
{
    return mConfiguration_.max_logical_port;
}

std::vector<std::string> TCPv6Transport::GetBindingInterfacesList()
{
    std::vector<std::string> vOutputInterfaces;
    if (IsInterfaceWhiteListEmpty())
    {
        vOutputInterfaces.push_back(s_IPv6AddressAny);
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

bool TCPv6Transport::IsLocatorAllowed(const Locator_t& locator) const
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }
    if (mInterfaceWhiteList.empty())
    {
        return true;
    }
    return IsInterfaceAllowed(IPLocator::toIPv6string(locator));
}

bool TCPv6Transport::IsInterfaceWhiteListEmpty() const
{
    return mInterfaceWhiteList.empty();
}

bool TCPv6Transport::IsInterfaceAllowed(const std::string& interface) const
{
    return IsInterfaceAllowed(asio::ip::address_v6::from_string(interface));
}

bool TCPv6Transport::IsInterfaceAllowed(const ip::address_v6& ip) const
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v6::any())
        return true;

    return find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}

LocatorList_t TCPv6Transport::NormalizeLocator(const Locator_t& locator)
{
    LocatorList_t list;

    if (IPLocator::isAny(locator))
    {
        std::vector<IPFinder::info_IP> locNames;
        GetIP6s(locNames);
        for (const auto& infoIP : locNames)
        {
            Locator_t newloc(locator);
            IPLocator::setIPv6(newloc, infoIP.locator);
            list.push_back(newloc);
        }
    }
    else
        list.push_back(locator);

    return list;
}

bool TCPv6Transport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_TCPv6);

    if (IPLocator::isLocal(locator))
    {
        return true;
    }

    for (auto localInterface : mCurrentInterfaces)
    {
        if (IPLocator::compareAddress(locator, localInterface.locator))
        {
            return true;
        }
    }

    return false;
}

bool TCPv6Transport::CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool TCPv6Transport::CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void TCPv6Transport::FillLocalIp(Locator_t& loc) const
{
    IPLocator::setIPv6(loc, "::1");
    loc.kind = LOCATOR_KIND_TCPv6;
}

ip::tcp::endpoint TCPv6Transport::GenerateEndpoint(const Locator_t& loc, uint16_t port) const
{
    asio::ip::address_v6::bytes_type remoteAddress;
    IPLocator::copyIPv6(loc, remoteAddress.data());
    return ip::tcp::endpoint(asio::ip::address_v6(remoteAddress), port);
}

ip::tcp::endpoint TCPv6Transport::GenerateLocalEndpoint(Locator_t& loc, uint16_t port) const
{
    return ip::tcp::endpoint(asio::ip::address_v6(locatorToNative(loc)), port);
}

ip::tcp::endpoint TCPv6Transport::GenerateEndpoint(uint16_t port) const
{
    return asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port);
}

asio::ip::tcp TCPv6Transport::GenerateProtocol() const
{
    return asio::ip::tcp::v6();
}

bool TCPv6Transport::IsInterfaceAllowed(const Locator_t& loc) const
{
    asio::ip::address_v6 ip = asio::ip::address_v6::from_string(IPLocator::toIPv6string(loc));
    return IsInterfaceAllowed(ip);
}

void TCPv6Transport::SetReceiveBufferSize(uint32_t size)
{
    mConfiguration_.receiveBufferSize = size;
}

void TCPv6Transport::SetSendBufferSize(uint32_t size)
{
    mConfiguration_.sendBufferSize = size;
}

void TCPv6Transport::EndpointToLocator(const ip::tcp::endpoint& endpoint, Locator_t& locator) const
{
    locator.kind = LOCATOR_KIND_TCPv6;
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    IPLocator::setIPv6(locator, ipBytes.data());
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
