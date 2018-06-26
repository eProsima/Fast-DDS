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
#include <fastrtps/rtps/network/ReceiverResource.h>
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

static bool IsAny(const Locator_t& locator)
{
    return locator.is_Any();
}

static asio::ip::address_v6::bytes_type locatorToNative(const Locator_t& locator)
{
    return {{locator.get_Address()[0],
        locator.get_Address()[1], locator.get_Address()[2], locator.get_Address()[3],
        locator.get_Address()[4], locator.get_Address()[5], locator.get_Address()[6],
        locator.get_Address()[7], locator.get_Address()[8], locator.get_Address()[9],
        locator.get_Address()[10], locator.get_Address()[11],locator.get_Address()[12],
        locator.get_Address()[13], locator.get_Address()[14], locator.get_Address()[15]}};
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
    defaultList.push_back(Locator_t(LOCATOR_KIND_UDPv6, "239.255.0.1", 0));
}

bool UDPv6Transport::CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const
{
    return lh.compare_IP6_address(rh);
}

bool UDPv6Transport::CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const
{
    return lh.compare_IP6_address_and_port(rh);
}

void UDPv6Transport::EndpointToLocator(ip::udp::endpoint& endpoint, Locator_t& locator)
{
    locator.set_port(endpoint.port());
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    locator.set_IP6_address(ipBytes.data());
}

void UDPv6Transport::FillLocalIp(Locator_t& loc)
{
    loc.set_IP6_address("::1");
}

const UDPTransportDescriptor* UDPv6Transport::GetConfiguration() const
{
    return &mConfiguration_;
}

ip::udp::endpoint UDPv6Transport::GenerateEndpoint(const Locator_t& loc, uint16_t port)
{
    asio::ip::address_v6::bytes_type remoteAddress;
    loc.copy_Address(remoteAddress.data());
    return ip::udp::endpoint(asio::ip::address_v6(remoteAddress), port);
}

asio::ip::udp::endpoint UDPv6Transport::GenerateAnyAddressEndpoint(uint16_t port)
{
    return ip::udp::endpoint(ip::address_v6::any(), port);
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

bool UDPv6Transport::OpenInputChannel(const Locator_t& locator, ReceiverResource* receiverResource,
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
        auto pChannelResource = mInputSockets.at(locator.get_physical_port());

        std::vector<IPFinder::info_IP> locNames;
        GetIP6sUniqueInterfaces(locNames);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v6::from_string(infoIP.name);
            try
            {
                pChannelResource->getSocket()->set_option(ip::multicast::join_group(ip::address_v6::from_string(locator.to_IP6_string()), ip.scope_id()));
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

bool UDPv6Transport::IsInterfaceAllowed(const ip::address_v6& ip)
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v6::any())
        return true;

    return  find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}


bool UDPv6Transport::OpenAndBindOutputSockets(const Locator_t& locator, SenderResource *senderResource)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    uint16_t port = locator.get_physical_port();

    try
    {
        if(IsAny(locator))
        {
            std::vector<IPFinder::info_IP> locNames;
            GetIP6s(locNames);
            // If there is no whitelist, we can simply open a generic output socket
            // and gain efficiency.
            if(mInterfaceWhiteList.empty())
            {
                eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(GenerateAnyAddressEndpoint(port), port);
                getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback( true ) );

                // If more than one interface, then create sockets for outbounding multicast.
                if(locNames.size() > 1)
                {
                    auto locIt = locNames.begin();

                    // Outbounding first interface with already created socket.
                    getSocketPtr(unicastSocket)->set_option(ip::multicast::outbound_interface(asio::ip::address_v6::from_string((*locIt).name).scope_id()));
                    mOutputSockets.push_back(new UDPChannelResource(unicastSocket));

                    // Create other socket for outbounding rest of interfaces.
                    for(++locIt; locIt != locNames.end(); ++locIt)
                    {
                        auto ip = asio::ip::address_v6::from_string((*locIt).name);
                        uint16_t new_port = 0;
                        eProsimaUDPSocket multicastSocket = OpenAndBindUnicastOutputSocket(asio::ip::udp::endpoint(ip, new_port), new_port);
                        getSocketPtr(multicastSocket)->set_option(ip::multicast::outbound_interface(ip.scope_id()));
                        UDPChannelResource *mSocket = new UDPChannelResource(multicastSocket);
                        mSocket->only_multicast_purpose(true);
                        mOutputSockets.push_back(mSocket);
                        // senderResource cannot be optimize in this cases
                    }
                }
                else
                {
                    // Multicast data will be sent for the only one interface.
                    UDPChannelResource *mSocket = new UDPChannelResource(unicastSocket);
                    mOutputSockets.push_back(mSocket);
                    if (senderResource != nullptr)
                    {
                        AssociateSenderToSocket(mSocket, senderResource);
                    }
                }
            }
            else
            {
                bool firstInterface = false;
                for (const auto& infoIP : locNames)
                {
                    auto ip = asio::ip::address_v6::from_string(infoIP.name);
                    if (IsInterfaceAllowed(ip))
                    {
                        eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(asio::ip::udp::endpoint(ip, port), port);
                        getSocketPtr(unicastSocket)->set_option(ip::multicast::outbound_interface(ip.scope_id()));
                        if(firstInterface)
                        {
                            getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback( true ) );
                            firstInterface = true;
                        }
                        mOutputSockets.push_back(new UDPChannelResource(unicastSocket));
                        // senderResource cannot be optimize in this cases
                    }
                }
            }
        }
        else
        {
            auto ip = asio::ip::address_v6(locatorToNative(locator));
            eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(asio::ip::udp::endpoint(ip, port), port);
            getSocketPtr(unicastSocket)->set_option(ip::multicast::outbound_interface(ip.scope_id()));
            getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback( true ) );

            UDPChannelResource *mSocket = new UDPChannelResource(unicastSocket);
            mOutputSockets.push_back(mSocket);
            if (senderResource != nullptr)
            {
                AssociateSenderToSocket(mSocket, senderResource);
            }
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv6 Error binding at port: (" << locator.get_physical_port() << ")" << " with msg: "<<e.what());

        for (auto& socket : mOutputSockets)
        {
            //auto it = mSocketToSenders.find(socket);
            //if (it != mSocketToSenders.end())
            //{
            //    auto& senders = mSocketToSenders.at(socket);
            //    for (auto& sender : senders)
            //    {
            //        sender->SetChannelResource(nullptr);
            //    }
            //}

            delete socket;
        }
        mOutputSockets.clear();
        return false;
    }

    return true;
}

LocatorList_t UDPv6Transport::NormalizeLocator(const Locator_t& locator)
{
	LocatorList_t list;

	if (locator.is_Any())
	{
		std::vector<IPFinder::info_IP> locNames;
		GetIP6s(locNames);
		for (const auto& infoIP : locNames)
		{
			Locator_t newloc(infoIP.locator);
			newloc.kind = locator.kind;
			newloc.set_port(locator.get_physical_port());
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

    if(locator.is_IP6_Local())
        return true;

    for(auto localInterface : currentInterfaces)
        if(localInterface.locator.compare_IP6_address(locator))
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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
