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
#include <fastrtps/transport/UDPv6Transport.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/rtps/network/SenderResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps{

static void GetIP6s(vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    IPFinder::getIPs(&locNames, return_loopback);
    // Controller out IP4
    auto new_end = remove_if(locNames.begin(),
            locNames.end(),
            [](IPFinder::info_IP ip){return ip.type != IPFinder::IP6 && ip.type != IPFinder::IP6_LOCAL;});
    locNames.erase(new_end, locNames.end());
}

static void GetIP6sUniqueInterfaces(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false)
{
    GetIP6s(locNames, return_loopback);
    std::sort(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool {return a.dev < b.dev;});
    auto new_end = std::unique(locNames.begin(), locNames.end(),
            [](const IPFinder::info_IP&  a, const IPFinder::info_IP& b) -> bool {return a.dev == b.dev;});
    locNames.erase(new_end, locNames.end());
}

static asio::ip::address_v6::bytes_type locatorToNative(const Locator_t& locator)
{
    return {{ IPLocator::getIPv6(locator)[0],
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
        IPLocator::getIPv6(locator)[15]}};
}

UDPv6Transport::UDPv6Transport(const UDPv6TransportDescriptor& descriptor)
    : mConfiguration_(descriptor)
{
    mTransportKind = LOCATOR_KIND_UDPv6;
    mSendBufferSize = descriptor.sendBufferSize;
    mReceiveBufferSize = descriptor.receiveBufferSize;
    for (const auto& interface : descriptor.interfaceWhiteList)
        mInterfaceWhiteList.emplace_back(ip::address_v6::from_string(interface));
}

UDPv6TransportDescriptor::UDPv6TransportDescriptor()
    : UDPTransportDescriptor()
{
}

UDPv6TransportDescriptor::UDPv6TransportDescriptor(const UDPv6TransportDescriptor& t)
    : UDPTransportDescriptor(t)
{
}

TransportInterface* UDPv6TransportDescriptor::create_transport() const
{
    return new UDPv6Transport(*this);
}

UDPv6Transport::UDPv6Transport()
{
    mTransportKind = LOCATOR_KIND_UDPv6;
}

UDPv6Transport::~UDPv6Transport()
{
    Clean();
}

void UDPv6Transport::AddDefaultOutputLocator(LocatorList_t &defaultList)
{
    // TODO What is the default IPv6 address?
    Locator_t temp;
    IPLocator::createLocator(LOCATOR_KIND_UDPv6, "239.255.0.1", 0, temp);
    defaultList.push_back(temp);
}

bool UDPv6Transport::CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const
{
    return IPLocator::compareAddress(lh, rh);
}

bool UDPv6Transport::CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const
{
    return IPLocator::compareAddressAndPhysicalPort(lh, rh);
}

void UDPv6Transport::EndpointToLocator(ip::udp::endpoint& endpoint, Locator_t& locator)
{
    IPLocator::setPhysicalPort(locator, endpoint.port());
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    IPLocator::setIPv6(locator, ipBytes.data());
}

void UDPv6Transport::FillLocalIp(Locator_t& loc)
{
    IPLocator::setIPv6(loc, "::1");
}

const UDPTransportDescriptor* UDPv6Transport::GetConfiguration() const
{
    return &mConfiguration_;
}

ip::udp::endpoint UDPv6Transport::GenerateEndpoint(const Locator_t& loc, uint16_t port)
{
    asio::ip::address_v6::bytes_type remoteAddress;
    IPLocator::copyIPv6(loc, remoteAddress.data());
    return ip::udp::endpoint(asio::ip::address_v6(remoteAddress), port);
}

asio::ip::udp::endpoint UDPv6Transport::GenerateAnyAddressEndpoint(uint16_t port)
{
    return ip::udp::endpoint(ip::address_v6::any(), port);
}

ip::udp::endpoint UDPv6Transport::GenerateEndpoint(const std::string& sIp, uint16_t port)
{
    return asio::ip::udp::endpoint(ip::address_v6::from_string(sIp), port);
}

ip::udp::endpoint UDPv6Transport::GenerateEndpoint(uint16_t port)
{
    return asio::ip::udp::endpoint(asio::ip::udp::v6(), port);
}

ip::udp::endpoint UDPv6Transport::GenerateLocalEndpoint(const Locator_t& loc, uint16_t port)
{
    return ip::udp::endpoint(asio::ip::address_v6(locatorToNative(loc)), port);
}

asio::ip::udp UDPv6Transport::GenerateProtocol() const
{
    return ip::udp::v6();
}

void UDPv6Transport::GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback)
{
    GetIP6s(locNames, return_loopback);
}

bool UDPv6Transport::OpenInputChannel(const Locator_t& locator, TransportReceiverInterface* receiver,
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
        // The multicast group will be joined silently, because we do not
        // want to return another resource.
        auto pChannelResource = mInputSockets.at(IPLocator::getPhysicalPort(locator));

        std::vector<IPFinder::info_IP> locNames;
        GetIP6sUniqueInterfaces(locNames);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v6::from_string(infoIP.name);
            try
            {
                pChannelResource->getSocket()->set_option(ip::multicast::join_group(
                        ip::address_v6::from_string(IPLocator::toIPv6string(locator)), ip.scope_id()));
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

bool UDPv6Transport::IsInterfaceAllowed(const std::string& interface)
{
    return IsInterfaceAllowed(asio::ip::address_v6::from_string(interface));
}

bool UDPv6Transport::IsInterfaceAllowed(const ip::address_v6& ip)
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v6::any())
        return true;

    return  find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}

bool UDPv6Transport::IsInterfaceWhiteListEmpty() const
{
    return mInterfaceWhiteList.empty();
}

LocatorList_t UDPv6Transport::NormalizeLocator(const Locator_t& locator)
{
	LocatorList_t list;

	if (IPLocator::isAny(locator))
	{
		std::vector<IPFinder::info_IP> locNames;
		GetIP6s(locNames);
		for (const auto& infoIP : locNames)
		{
			Locator_t newloc(infoIP.locator);
			newloc.kind = locator.kind;
			IPLocator::setPhysicalPort(newloc, IPLocator::getPhysicalPort(locator));
			list.push_back(newloc);
		}
	}
	else
		list.push_back(locator);

	return list;
}

bool UDPv6Transport::is_local_locator(const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_UDPv4);

    if(IPLocator::isLocal(locator))
        return true;

    for(auto localInterface : currentInterfaces)
        if(IPLocator::compareAddress(localInterface.locator, locator))
            return true;

    return false;
}

void UDPv6Transport::SetReceiveBufferSize(uint32_t size)
{
    mConfiguration_.receiveBufferSize = size;
}

void UDPv6Transport::SetSendBufferSize(uint32_t size)
{
    mConfiguration_.sendBufferSize = size;
}

void UDPv6Transport::SetSocketOutbountInterface(eProsimaUDPSocket& socket, const std::string& sIp)
{
	getSocketPtr(socket)->set_option(ip::multicast::outbound_interface(asio::ip::address_v6::from_string(sIp).scope_id()));
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
