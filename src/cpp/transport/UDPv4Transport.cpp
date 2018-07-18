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
	if (locator.has_IP4_WAN_address())
	{
		return{ { locator.get_IP4_WAN_address(0),
			locator.get_IP4_WAN_address(1), locator.get_IP4_WAN_address(2), locator.get_IP4_WAN_address(3) } };
	}
	else
	{
		return{ { locator.get_IP4_address(0),
			locator.get_IP4_address(1), locator.get_IP4_address(2), locator.get_IP4_address(3) } };
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

void UDPv4Transport::AddDefaultOutputLocator(LocatorList_t &defaultList)
{
    defaultList.push_back(Locator_t(LOCATOR_KIND_UDPv4, "239.255.0.1", mConfiguration_.m_output_udp_socket));
}

bool UDPv4Transport::CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const
{
    return lh.compare_IP4_address(rh);
}

bool UDPv4Transport::CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const
{
    return lh.compare_IP4_address_and_port(rh);
}

void UDPv4Transport::EndpointToLocator(ip::udp::endpoint& endpoint, Locator_t& locator)
{
    locator.get_physical_port_by_ref() = endpoint.port();
    auto ipBytes = endpoint.address().to_v4().to_bytes();
    locator.set_IP4_address(ipBytes.data());
}

void UDPv4Transport::FillLocalIp(Locator_t& loc)
{
    loc.set_IP4_address("127.0.0.1");
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
    loc.copy_IP4_address(remoteAddress.data());
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

bool UDPv4Transport::OpenInputChannel(const Locator_t& locator, ReceiverResource* receiverResource,
    uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    bool success = false;

    if (!IsInputChannelOpen(locator))
        success = OpenAndBindInputSockets(locator, receiverResource, locator.is_Multicast(), maxMsgSize);

    if (locator.is_Multicast() && IsInputChannelOpen(locator))
    {
        // The multicast group will be joined silently, because we do not
        // want to return another resource.
        auto& channelResource = mInputSockets.at(locator.get_physical_port());

        std::vector<IPFinder::info_IP> locNames;
        GetIP4sUniqueInterfaces(locNames, true);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v4::from_string(infoIP.name);
            try
            {
                channelResource->getSocket()->set_option(ip::multicast::join_group(ip::address_v4::from_string(locator.to_IP4_string()), ip));
            }
            catch(std::system_error& ex)
            {
                (void)ex;
                logWarning(RTPS_MSG_OUT, "Error joining multicast group on " << ip << ": "<< ex.what());
            }
        }
    }

    return success;
}

bool UDPv4Transport::IsInterfaceAllowed(const std::string& interface)
{
    return IsInterfaceAllowed(asio::ip::address_v4::from_string(interface));
}

bool UDPv4Transport::IsInterfaceAllowed(const ip::address_v4& ip)
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

LocatorList_t UDPv4Transport::NormalizeLocator(const Locator_t& locator)
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

bool UDPv4Transport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_UDPv4);

    if(locator.is_IP4_Local())
        return true;

    for(auto localInterface : currentInterfaces)
        if(locator.compare_IP4_address(localInterface.locator))
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

void UDPv4Transport::SetSocketOutbountInterface(eProsimaUDPSocket& socket, const std::string& sIp)
{
	getSocketPtr(socket)->set_option(ip::multicast::outbound_interface(asio::ip::address_v4::from_string(sIp)));
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
