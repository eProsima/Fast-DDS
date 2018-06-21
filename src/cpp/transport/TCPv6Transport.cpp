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
}

static asio::ip::address_v6::bytes_type locatorToNative(Locator_t& locator)
{
    return{ {locator.get_IP6_address()[0], locator.get_IP6_address()[1], locator.get_IP6_address()[2],
        locator.get_IP6_address()[3] , locator.get_IP6_address()[4] , locator.get_IP6_address()[5],
        locator.get_IP6_address()[6] , locator.get_IP6_address()[7] , locator.get_IP6_address()[8],
        locator.get_IP6_address()[9] , locator.get_IP6_address()[10] , locator.get_IP6_address()[11],
        locator.get_IP6_address()[12] , locator.get_IP6_address()[13] , locator.get_IP6_address()[14],
        locator.get_IP6_address()[15] } };
}

TCPv6Transport::TCPv6Transport(const TCPv6TransportDescriptor& descriptor)
    : mConfiguration_(descriptor)
{
    mTransportKind = LOCATOR_KIND_TCPv6;
    for (const auto& interface : descriptor.interfaceWhiteList)
    {
        mInterfaceWhiteList.emplace_back(ip::address_v6::from_string(interface));
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

void TCPv6Transport::AddDefaultOutputLocator(LocatorList_t& defaultList)
{
    std::unique_lock<std::recursive_mutex> scoped(mSocketsMapMutex);
    if (mConfiguration_.listening_ports.size() > 0)
    {
        for (auto it = mConfiguration_.listening_ports.begin(); it != mConfiguration_.listening_ports.end(); ++it)
        {
            defaultList.push_back(Locator_t(LOCATOR_KIND_TCPv6, "::1", *it));
        }
    }
    else if (mSocketConnectors.size() > 0)
    {
        defaultList.push_back(mSocketConnectors.begin()->first);
    }
    else if (mBoundOutputSockets.size() > 0)
    {
        defaultList.push_back(mBoundOutputSockets.begin()->first);
    }
    else
    {
        defaultList.push_back(Locator_t(LOCATOR_KIND_TCPv6, "::1", 0));
    }
}

const TCPTransportDescriptor* TCPv6Transport::GetConfiguration() const
{
    return &mConfiguration_;
}

void TCPv6Transport::GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback)
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

bool TCPv6Transport::IsInterfaceAllowed(const ip::address_v6& ip)
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

    if (locator.is_Any())
    {
        std::vector<IPFinder::info_IP> locNames;
        GetIP6s(locNames);
        for (const auto& infoIP : locNames)
        {
            Locator_t newloc(locator);
            newloc.set_IP6_address(infoIP.locator);
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

    if (locator.is_IP6_Local())
    {
        return true;
    }

    for (auto localInterface : mCurrentInterfaces)
    {
        if (locator.compare_IP6_address(localInterface.locator))
        {
            return true;
        }
    }

    return false;
}

bool TCPv6Transport::CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const
{
    return lh.compare_IP6_address(rh);
}

bool TCPv6Transport::CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const
{
    return lh.compare_IP6_address_and_port(rh);
}

void TCPv6Transport::FillLocalIp(Locator_t& loc)
{
    loc.set_IP4_address("::1");
}

ip::tcp::endpoint TCPv6Transport::GenerateEndpoint(const Locator_t& loc, uint16_t port)
{
    asio::ip::address_v6::bytes_type remoteAddress;
    loc.copy_Address(remoteAddress.data());
    return ip::tcp::endpoint(asio::ip::address_v6(remoteAddress), port);
}

ip::tcp::endpoint TCPv6Transport::GenerateLocalEndpoint(Locator_t& loc, uint16_t port)
{
    return ip::tcp::endpoint(asio::ip::address_v6(locatorToNative(loc)), port);
}

ip::tcp::endpoint TCPv6Transport::GenerateEndpoint(uint16_t port)
{
    return asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port);
}

bool TCPv6Transport::IsInterfaceAllowed(const Locator_t& loc)
{
    asio::ip::address_v6 ip = asio::ip::make_address_v6(loc.to_IP6_string());
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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
