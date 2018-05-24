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

#include <fastrtps/transport/UDPv6Transport.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/rtps/network/ReceiverResource.h>

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

static asio::ip::address_v6::bytes_type locatorToNative(Locator_t& locator)
{
    return {{locator.get_Address()[0],
        locator.get_Address()[1], locator.get_Address()[2], locator.get_Address()[3],
        locator.get_Address()[4], locator.get_Address()[5], locator.get_Address()[6],
        locator.get_Address()[7], locator.get_Address()[8], locator.get_Address()[9],
        locator.get_Address()[10], locator.get_Address()[11],locator.get_Address()[12],
        locator.get_Address()[13], locator.get_Address()[14], locator.get_Address()[15]}};
}

UDPv6Transport::UDPv6Transport(const UDPv6TransportDescriptor& descriptor):
    mConfiguration_(descriptor),
    mSendBufferSize(descriptor.sendBufferSize),
    mReceiveBufferSize(descriptor.receiveBufferSize)
    {
        for (const auto& interface : descriptor.interfaceWhiteList)
           mInterfaceWhiteList.emplace_back(ip::address_v6::from_string(interface));
    }

UDPv6TransportDescriptor::UDPv6TransportDescriptor():
    TransportDescriptorInterface(s_maximumMessageSize)
{
}

UDPv6TransportDescriptor::UDPv6TransportDescriptor(const UDPv6TransportDescriptor& t) :
    TransportDescriptorInterface(t)
{
}

TransportInterface* UDPv6TransportDescriptor::create_transport() const
{
    return new UDPv6Transport(*this);
}

UDPv6Transport::~UDPv6Transport()
{
    if(ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }
}

void UDPv6Transport::AddDefaultLocator(LocatorList_t &defaultList)
{
    defaultList.push_back(Locator_t(LOCATOR_KIND_UDPv6, 0));
}

bool UDPv6Transport::init()
{
    if(mConfiguration_.sendBufferSize == 0 || mConfiguration_.receiveBufferSize == 0)
    {
        // Check system buffer sizes.
        ip::udp::socket socket(mService);
        socket.open(ip::udp::v6());

        if(mConfiguration_.sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            mConfiguration_.sendBufferSize = option.value();

            if(mConfiguration_.sendBufferSize < s_minimumSocketBuffer)
            {
                mConfiguration_.sendBufferSize = s_minimumSocketBuffer;
                mSendBufferSize = s_minimumSocketBuffer;
            }
        }

        if(mConfiguration_.receiveBufferSize == 0)
        {
            socket_base::receive_buffer_size option;
            socket.get_option(option);
            mConfiguration_.receiveBufferSize = option.value();

            if(mConfiguration_.receiveBufferSize < s_minimumSocketBuffer)
            {
                mConfiguration_.receiveBufferSize = s_minimumSocketBuffer;
                mReceiveBufferSize = s_minimumSocketBuffer;
            }
        }
    }

    if(mConfiguration_.maxMessageSize > s_maximumMessageSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if(mConfiguration_.maxMessageSize > mConfiguration_.sendBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than sendBufferSize");
        return false;
    }

    if(mConfiguration_.maxMessageSize > mConfiguration_.receiveBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than receiveBufferSize");
        return false;
    }

    // TODO(Ricardo) Create an event that update this list.
    GetIP6s(currentInterfaces);

    auto ioServiceFunction = [&]()
    {
        io_service::work work(mService);
        mService.run();
    };
    ioServiceThread.reset(new std::thread(ioServiceFunction));

    return true;
}

bool UDPv6Transport::IsInputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.get_physical_port()) != mInputSockets.end());
}

bool UDPv6Transport::IsOutputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    return mOutputSockets.find(locator.get_physical_port()) != mOutputSockets.end();
}

bool UDPv6Transport::OpenOutputChannel(Locator_t& locator)
{
    if (!IsLocatorSupported(locator) || IsOutputChannelOpen(locator))
        return false;

    return OpenAndBindOutputSockets(locator);
}

static bool IsMulticastAddress(const Locator_t& locator)
{
    return locator.is_Multicast();
}

bool UDPv6Transport::OpenInputChannel(const Locator_t& locator, ReceiverResource* receiverResource,
    uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    bool success = false;

    if (!IsInputChannelOpen(locator))
        success = OpenAndBindInputSockets(locator, receiverResource, IsMulticastAddress(locator), maxMsgSize);

    if (IsMulticastAddress(locator) && IsInputChannelOpen(locator))
    {
        // The multicast group will be joined silently, because we do not
        // want to return another resource.
        auto& socketInfo = mInputSockets.at(locator.get_physical_port());

        std::vector<IPFinder::info_IP> locNames;
        GetIP6sUniqueInterfaces(locNames);
        for (const auto& infoIP : locNames)
        {
            auto ip = asio::ip::address_v6::from_string(infoIP.name);
            try
            {
                socketInfo->getSocket()->set_option(ip::multicast::join_group(ip::address_v6::from_string(locator.to_IP6_string()), ip.scope_id()));
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

bool UDPv6Transport::CloseOutputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(locator))
        return false;

    auto& sockets = mOutputSockets.at(locator.get_physical_port());
    for (auto& socket : sockets)
    {
        socket.getSocket()->cancel();
        socket.getSocket()->close();
    }

    mOutputSockets.erase(locator.get_physical_port());

    return true;
}

bool UDPv6Transport::CloseInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsInputChannelOpen(locator))
        return false;


    auto& socketInfo = mInputSockets.at(locator.get_physical_port());
    socketInfo->getSocket()->cancel();
    socketInfo->getSocket()->close();

    mInputSockets.erase(locator.get_physical_port());
    return true;
}

bool UDPv6Transport::ReleaseInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsInputChannelOpen(locator))
        return false;

    try
    {
    	auto& socket = mInputSockets.at(locator.port);
        //getSocketPtr(socket)->open(ip::udp::v6());
        auto destinationEndpoint = ip::udp::endpoint(asio::ip::address_v6(locatorToNative(locator)),
                static_cast<uint16_t>(locator.port));
        getSocketPtr(socket)->send_to(asio::buffer("EPRORTPSCLOSE", 13), destinationEndpoint);
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        return false;
    }

    return true;
}

bool UDPv6Transport::IsInterfaceAllowed(const ip::address_v6& ip)
{
    if (mInterfaceWhiteList.empty())
        return true;

    if (ip == ip::address_v6::any())
        return true;

    return  find(mInterfaceWhiteList.begin(), mInterfaceWhiteList.end(), ip) != mInterfaceWhiteList.end();
}


bool UDPv6Transport::OpenAndBindOutputSockets(Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);

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
                eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(ip::address_v6::any(), locator.get_physical_port());
                getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback( true ) );

                // If more than one interface, then create sockets for outbounding multicast.
                if(locNames.size() > 1)
                {
                    auto locIt = locNames.begin();

                    // Outbounding first interface with already created socket.
                    getSocketPtr(unicastSocket)->set_option(ip::multicast::outbound_interface(asio::ip::address_v6::from_string((*locIt).name).scope_id()));
                    mOutputSockets[locator.get_physical_port()].push_back(UDPSocketInfo(unicastSocket));

                    // Create other socket for outbounding rest of interfaces.
                    for(++locIt; locIt != locNames.end(); ++locIt)
                    {
                        auto ip = asio::ip::address_v6::from_string((*locIt).name);
                        uint16_t new_port = 0;
                        eProsimaUDPSocket multicastSocket = OpenAndBindUnicastOutputSocket(ip, new_port);
                        getSocketPtr(multicastSocket)->set_option(ip::multicast::outbound_interface(ip.scope_id()));
                        UDPSocketInfo mSocket(multicastSocket);
                        mSocket.only_multicast_purpose(true);
                        mOutputSockets[locator.get_physical_port()].push_back(std::move(mSocket));
                    }
                }
                else
                {
                    // Multicast data will be sent for the only one interface.
                    mOutputSockets[locator.get_physical_port()].push_back(UDPSocketInfo(unicastSocket));
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
                        eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.get_physical_port());
                        getSocketPtr(unicastSocket)->set_option(ip::multicast::outbound_interface(ip.scope_id()));
                        if(firstInterface)
                        {
                            getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback( true ) );
                            firstInterface = true;
                        }
                        mOutputSockets[locator.get_physical_port()].push_back(UDPSocketInfo(unicastSocket));
                    }
                }
            }
        }
        else
        {
            auto ip = asio::ip::address_v6(locatorToNative(locator));
            eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(ip, locator.get_physical_port());
            getSocketPtr(unicastSocket)->set_option(ip::multicast::outbound_interface(ip.scope_id()));
            getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback( true ) );
            mOutputSockets[locator.get_physical_port()].push_back(UDPSocketInfo(unicastSocket));
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv6 Error binding at port: (" << locator.get_physical_port() << ")" << " with msg: "<<e.what());
        mOutputSockets.erase(locator.get_physical_port());
        return false;
    }

    return true;
}

bool UDPv6Transport::OpenAndBindInputSockets(const Locator_t& locator, ReceiverResource* receiverResource,
    bool is_multicast, uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    try
    {
        eProsimaUDPSocket unicastSocket = OpenAndBindInputSocket(locator.get_physical_port(), is_multicast);
        UDPSocketInfo* socketInfo = new UDPSocketInfo(unicastSocket, maxMsgSize);
        socketInfo->SetMessageReceiver(receiverResource->CreateMessageReceiver());
        std::thread* newThread = new std::thread(&UDPv6Transport::performListenOperation, this, socketInfo, locator);
        socketInfo->SetThread(newThread);
        mInputSockets.emplace(locator.get_physical_port(), socketInfo);
    }
    catch (asio::error_code const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPv6 Error binding at port: (" << locator.get_physical_port() << ")" << " with msg: "<<e.message() );
        mInputSockets.erase(locator.get_physical_port());
        return false;
    }

    return true;
}

void UDPv6Transport::performListenOperation(UDPSocketInfo* pSocketInfo, Locator_t input_locator)
{
    Locator_t remoteLocator;
    while (pSocketInfo->IsAlive())
    {
        // Blocking receive.
        auto msg = pSocketInfo->GetMessageBuffer();
        CDRMessage::initCDRMsg(&msg);
        if (!Receive(msg.buffer, msg.max_size, msg.length, pSocketInfo, remoteLocator))
            continue;

        // Processes the data through the CDR Message interface.
        pSocketInfo->GetMessageReceiver()->processCDRMsg(mConfiguration_.rtpsParticipantGuidPrefix, &input_locator,
            &pSocketInfo->GetMessageBuffer());
    }
}

eProsimaUDPSocket UDPv6Transport::OpenAndBindUnicastOutputSocket(const ip::address_v6& ipAddress, uint16_t& port)
{
    eProsimaUDPSocket socket = createUDPSocket(mService);
    getSocketPtr(socket)->open(ip::udp::v6());
    if(mSendBufferSize != 0)
        getSocketPtr(socket)->set_option(socket_base::send_buffer_size(mSendBufferSize));
    getSocketPtr(socket)->set_option(ip::multicast::hops(mConfiguration_.TTL));

    ip::udp::endpoint endpoint(ipAddress, port);
    getSocketPtr(socket)->bind(endpoint);

    if(port == 0)
        port = getSocketPtr(socket)->local_endpoint().port();

    return socket;
}

eProsimaUDPSocket UDPv6Transport::OpenAndBindInputSocket(uint16_t port, bool is_multicast)
{
    eProsimaUDPSocket socket = createUDPSocket(mService);
    getSocketPtr(socket)->open(ip::udp::v6());
    if(mReceiveBufferSize != 0)
        getSocketPtr(socket)->set_option(socket_base::receive_buffer_size(mReceiveBufferSize));
    if(is_multicast)
        getSocketPtr(socket)->set_option(ip::udp::socket::reuse_address( true ) );
    ip::udp::endpoint endpoint(ip::address_v6::any(), port);
    getSocketPtr(socket)->bind(endpoint);

    return socket;
}

bool UDPv6Transport::DoLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return left.get_port() == right.get_port();
}

bool UDPv6Transport::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == LOCATOR_KIND_UDPv6;
}

Locator_t UDPv6Transport::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
        return false;

    // All remotes are equally mapped to from the local [0:0:0:0:0:0:0:0]:port (main output channel).
    Locator_t mainLocal(remote);
    mainLocal.set_Invalid_Address();
    return remote;
}

//! Sets the ID of the participant that has created the transport.
void UDPv6Transport::SetParticipantGUIDPrefix(const GuidPrefix_t& prefix)
{
    mConfiguration_.rtpsParticipantGuidPrefix = prefix;
}

bool UDPv6Transport::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(localLocator) ||
            sendBufferSize > mConfiguration_.sendBufferSize)
        return false;

    bool success = false;
    bool is_multicast_remote_address = IsMulticastAddress(remoteLocator);

    auto& sockets = mOutputSockets.at(localLocator.get_physical_port());
    for (auto& socket : sockets)
    {
        if(is_multicast_remote_address || !socket.only_multicast_purpose())
            success |= SendThroughSocket(sendBuffer, sendBufferSize, remoteLocator, getRefFromPtr(socket.getSocket()));
    }

    return success;
}

static Locator_t EndpointToLocator(ip::udp::endpoint& endpoint)
{
    Locator_t locator;

    locator.set_port(endpoint.port());
    auto ipBytes = endpoint.address().to_v6().to_bytes();
    locator.set_IP6_address(ipBytes.data());

    return locator;
}

bool UDPv6Transport::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
        SocketInfo* socketInfo, Locator_t& remoteLocator)
{

    if (!socketInfo->IsAlive())
        return false;

    ip::udp::endpoint senderEndpoint;
    eProsimaSocketUDP* socket = nullptr;

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

        socket = &mInputSockets.at(localLocator.port);
    }

    if(socket != nullptr)
    {
        size_t bytes = getSocketPtr(socket)->receive_from(asio::buffer(receiveBuffer, receiveBufferCapacity), senderEndpoint);

        receiveBufferSize = static_cast<uint32_t>(bytes);

        if(receiveBufferSize > 0)
        {
            if(receiveBufferSize == 13 && memcmp(receiveBuffer, "EPRORTPSCLOSE", 13) == 0)
            {
                return false;
            }

            remoteLocator = EndpointToLocator(senderEndpoint);
        }
    }

    return (receiveBufferSize > 0);
}

bool UDPv6Transport::SendThroughSocket(const octet* sendBuffer,
        uint32_t sendBufferSize,
        const Locator_t& remoteLocator,
        eProsimaUDPSocket& socket)
{
    asio::ip::address_v6::bytes_type remoteAddress;
    //memcpy(&remoteAddress, remoteLocator.get_Address(), sizeof(remoteAddress));
    remoteLocator.copy_Address(remoteAddress.data());
    auto destinationEndpoint = ip::udp::endpoint(asio::ip::address_v6(remoteAddress), static_cast<uint16_t>(remoteLocator.get_port()));
    size_t bytesSent = 0;
    logInfo(RTPS_MSG_OUT,"UDPv6: " << sendBufferSize << " bytes TO endpoint: " << destinationEndpoint
            << " FROM " << getSocketPtr(socket)->local_endpoint());

    try
    {
        bytesSent = getSocketPtr(socket)->send_to(asio::buffer(sendBuffer, sendBufferSize), destinationEndpoint);
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        return false;
    }

    (void) bytesSent;
    logInfo (RTPS_MSG_OUT,"SENT " << bytesSent);
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

struct MultiUniLocatorsLinkage
{
    MultiUniLocatorsLinkage(LocatorList_t&& m, LocatorList_t&& u) :
        multicast(std::move(m)), unicast(std::move(u)) {}

    LocatorList_t multicast;
    LocatorList_t unicast;
};

LocatorList_t UDPv6Transport::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
{
    LocatorList_t multicastResult, unicastResult;
    std::vector<MultiUniLocatorsLinkage> pendingLocators;

    for(auto& locatorList : locatorLists)
    {
        LocatorListConstIterator it = locatorList.begin();
        bool multicastDefined = false;
        LocatorList_t pendingMulticast, pendingUnicast;

        while(it != locatorList.end())
        {
            if(IsMulticastAddress(*it))
            {
                assert((*it).kind == LOCATOR_KIND_UDPv6);

                // If the multicast locator is already chosen, not choose any unicast locator.
                if(multicastResult.contains(*it))
                {
                    multicastDefined = true;
                    pendingUnicast.clear();
                }
                else
                {
                    // Search the multicast locator in pending locators.
                    auto pending_it = pendingLocators.begin();
                    bool found = false;

                    while(pending_it != pendingLocators.end())
                    {
                        if((*pending_it).multicast.contains(*it))
                        {
                            // Multicast locator was found, add it to final locators.
                            multicastResult.push_back((*pending_it).multicast);
                            pendingLocators.erase(pending_it);

                            // Not choose any unicast
                            multicastDefined = true;
                            pendingUnicast.clear();
                            found = true;

                            break;
                        }

                        ++pending_it;
                    };

                    // If not found, store as pending multicast.
                    if(!found)
                        pendingMulticast.push_back(*it);
                }
            }
            else
            {
                if(!multicastDefined)
                {
                    // Check is local interface.
                    auto localInterface = currentInterfaces.begin();
                    for (; localInterface != currentInterfaces.end(); ++localInterface)
                    {
                        //if(memcmp(localInterface->locator.address, it->address, 16) == 0)
                        if(localInterface->locator.compare_IP6_address(*it))
                        {
                            // Loopback locator
                            Locator_t loopbackLocator;
                            loopbackLocator.set_IP6_address(0, 0, 0, 0, 0, 0, 0, 1);
                            loopbackLocator.set_port(it->get_physical_port());
                            pendingUnicast.push_back(loopbackLocator);
                            break;
                        }
                    }

                    if(localInterface == currentInterfaces.end())
                        pendingUnicast.push_back(*it);
                }
            }

            ++it;
        }

        if(pendingMulticast.size() == 0)
        {
            unicastResult.push_back(pendingUnicast);
        }
        else if(pendingUnicast.size() == 0)
        {
            multicastResult.push_back(pendingMulticast);
        }
        else
        {
            pendingLocators.push_back(MultiUniLocatorsLinkage(std::move(pendingMulticast), std::move(pendingUnicast)));
        }
    }

    LocatorList_t result(std::move(unicastResult));
    result.push_back(multicastResult);

    // Store pending unicast locators
    for(auto link : pendingLocators)
        result.push_back(link.unicast);

    return result;
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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
