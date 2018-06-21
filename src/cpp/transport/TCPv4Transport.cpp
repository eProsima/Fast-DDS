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
#include <fastrtps/transport/TCPv4Transport.h>
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
#include <fastrtps/transport/TCPv4TransportDescriptor.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

static void GetIP4s(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    auto new_end = remove_if(locNames.begin(),
        locNames.end(),
        [](IPFinder::info_IP ip) {return ip.type != IPFinder::IP4 && ip.type != IPFinder::IP4_LOCAL; });
    locNames.erase(new_end, locNames.end());
}

static asio::ip::address_v4::bytes_type locatorToNative(Locator_t& locator)
{
    if (locator.has_IP4_WAN_address())
    {
        return{ {locator.get_IP4_WAN_address()[0],
            locator.get_IP4_WAN_address()[1], locator.get_IP4_WAN_address()[2], locator.get_IP4_WAN_address()[3]} };
    }
    else
    {
        return{ {locator.get_IP4_address()[0],
            locator.get_IP4_address()[1], locator.get_IP4_address()[2], locator.get_IP4_address()[3]} };
    }
}

TCPv4Transport::TCPv4Transport(const TCPv4TransportDescriptor& descriptor)
    : mConfiguration_(descriptor)
{
    mTransportKind = LOCATOR_KIND_TCPv4;
    for (const auto& interface : descriptor.interfaceWhiteList)
    {
        mInterfaceWhiteList.emplace_back(ip::address_v4::from_string(interface));
    }
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor()
    : TCPTransportDescriptor()
{
    memset(wan_addr, 0, 4);
}

TCPv4TransportDescriptor::TCPv4TransportDescriptor(const TCPv4TransportDescriptor& t)
    : TCPTransportDescriptor(t)
{
    memcpy(wan_addr, t.wan_addr, 4);
}

TransportInterface* TCPv4TransportDescriptor::create_transport() const
{
    return new TCPv4Transport(*this);
}

TCPv4Transport::TCPv4Transport()
{
    mTransportKind = LOCATOR_KIND_TCPv4;
}

TCPv4Transport::~TCPv4Transport()
{
}

void TCPv4Transport::AddDefaultOutputLocator(LocatorList_t& defaultList)
{
    std::unique_lock<std::recursive_mutex> scoped(mSocketsMapMutex);
    if (mConfiguration_.listening_ports.size() > 0)
    {
        for (auto it = mConfiguration_.listening_ports.begin(); it != mConfiguration_.listening_ports.end(); ++it)
        {
            defaultList.push_back(Locator_t(LOCATOR_KIND_TCPv4, "127.0.0.1", *it));
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
        defaultList.push_back(Locator_t(LOCATOR_KIND_TCPv4, "127.0.0.1", 0));
    }
}

const TCPTransportDescriptor* TCPv4Transport::GetConfiguration() const
{
    return &mConfiguration_;
}

void TCPv4Transport::GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback)
{
    GetIP4s(locNames, return_loopback);
}

uint16_t TCPv4Transport::GetLogicalPortIncrement() const
{
    return mConfiguration_.logical_port_increment;
}

uint16_t TCPv4Transport::GetLogicalPortRange() const
{
    return mConfiguration_.logical_port_range;
}

uint16_t TCPv4Transport::GetMaxLogicalPort() const
{
    return mConfiguration_.max_logical_port;
}

bool TCPv4Transport::IsInterfaceAllowed(const ip::address_v4& ip)
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v4::any())
        return true;

    return find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}

LocatorList_t TCPv4Transport::NormalizeLocator(const Locator_t& locator)
{
    LocatorList_t list;

    if (locator.is_Any())
    {
        std::vector<IPFinder::info_IP> locNames;
        GetIP4s(locNames);
        for (const auto& infoIP : locNames)
        {
            Locator_t newloc(locator);
            newloc.set_IP4_address(infoIP.locator);
            list.push_back(newloc);
        }
    }
    else
        list.push_back(locator);

    return list;
}

bool TCPv4Transport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_TCPv4);

    if (locator.is_IP4_Local())
    {
        return true;
    }

    for (auto localInterface : mCurrentInterfaces)
    {
        if (locator.compare_IP4_address(localInterface.locator))
        {
            return true;
        }
    }

    return false;
}

bool TCPv4Transport::CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const
{
    return lh.compare_IP4_address(rh);
}

bool TCPv4Transport::CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const
{
    return lh.compare_IP4_address_and_port(rh);
}

void TCPv4Transport::FillLocalIp(Locator_t& loc)
{
    loc.set_IP4_address("127.0.0.1");
}

ip::tcp::endpoint TCPv4Transport::GenerateEndpoint(const Locator_t& loc, uint16_t port)
{
    asio::ip::address_v4::bytes_type remoteAddress;
    loc.copy_IP4_address(remoteAddress.data());
    return ip::tcp::endpoint(asio::ip::address_v4(remoteAddress), port);
}

ip::tcp::endpoint TCPv4Transport::GenerateLocalEndpoint(Locator_t& loc, uint16_t port)
{
    return ip::tcp::endpoint(asio::ip::address_v4(locatorToNative(loc)), port);
}

ip::tcp::endpoint TCPv4Transport::GenerateEndpoint(uint16_t port)
{
    return asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
}

bool TCPv4Transport::IsInterfaceAllowed(const Locator_t& loc)
{
    asio::ip::address_v4 ip = asio::ip::make_address_v4(loc.to_IP4_string());
    return IsInterfaceAllowed(ip);
}

void TCPv4Transport::SetReceiveBufferSize(uint32_t size)
{
    mConfiguration_.receiveBufferSize = size;
}

void TCPv4Transport::SetSendBufferSize(uint32_t size)
{
    mConfiguration_.sendBufferSize = size;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
