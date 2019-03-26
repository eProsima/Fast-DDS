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
#include <fastrtps/transport/UDPTransportInterface.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastrtps{
namespace rtps {

struct MultiUniLocatorsLinkage
{
    MultiUniLocatorsLinkage(LocatorList_t&& m, LocatorList_t&& u)
        : multicast(std::move(m))
        , unicast(std::move(u))
    {
    }

    LocatorList_t multicast;
    LocatorList_t unicast;
};

UDPTransportDescriptor::UDPTransportDescriptor()
    : SocketTransportDescriptor(s_maximumMessageSize, s_maximumInitialPeersRange)
    , m_output_udp_socket(0)
{
}

UDPTransportDescriptor::UDPTransportDescriptor(const UDPTransportDescriptor& t)
    : SocketTransportDescriptor(t)
    , m_output_udp_socket(t.m_output_udp_socket)
{
}

UDPTransportInterface::UDPTransportInterface()
: mSendBufferSize(0)
, mReceiveBufferSize(0)
{
}

UDPTransportInterface::~UDPTransportInterface()
{
}

void UDPTransportInterface::Clean()
{
    assert(mInputSockets.size() == 0);
    assert(mOutputSockets.size() == 0);
}

bool UDPTransportInterface::CloseInputChannel(const Locator_t& locator)
{
    std::vector<UDPChannelResource*> channel_resources;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
        if (!IsInputChannelOpen(locator))
            return false;

        channel_resources = std::move(mInputSockets.at(IPLocator::getPhysicalPort(locator)));
        mInputSockets.erase(IPLocator::getPhysicalPort(locator));

    }

    std::map<UDPChannelResource*, asio::ip::address> addresses;
    // It may sound redundant, but we must mark all the related channel to be killed first.
    // Mostly in Windows, but in Linux can happen too, if we access to the endpoint
    // of an already closed socket we get an exception. So we store the interface address to
    // be used in the ReleaseInputChannel call later.
    for (UDPChannelResource* channel_resource : channel_resources)
    {
        if (channel_resource->IsAlive())
        {
            addresses[channel_resource] = channel_resource->getSocket()->local_endpoint().address();
        }
        else
        {
            addresses[channel_resource] = asio::ip::address();
        }
        channel_resource->Disable();
    }

    // Then we release the channels
    for (UDPChannelResource* channel : channel_resources)
    {
        ReleaseInputChannel(locator, addresses[channel]);
        channel->getSocket()->cancel();
        channel->getSocket()->close();
        delete channel;
    }

    return true;
}

bool UDPTransportInterface::CloseOutputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(locator))
        return false;

    for (auto& socket : mOutputSockets)
    {
        socket->getSocket()->cancel();
        socket->getSocket()->close();

        delete socket;
    }
    mOutputSockets.clear();

    return true;
}

bool UDPTransportInterface::DoInputLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return IPLocator::getPhysicalPort(left) == IPLocator::getPhysicalPort(right);
}

bool UDPTransportInterface::DoOutputLocatorsMatch(const Locator_t&, const Locator_t&) const
{
    return true;
}

bool UDPTransportInterface::init()
{
    if (GetConfiguration()->sendBufferSize == 0 || GetConfiguration()->receiveBufferSize == 0)
    {
        // Check system buffer sizes.
        ip::udp::socket socket(mService);
        socket.open(GenerateProtocol());

        if (GetConfiguration()->sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            SetSendBufferSize(option.value());

            if (GetConfiguration()->sendBufferSize < s_minimumSocketBuffer)
            {
                SetSendBufferSize(s_minimumSocketBuffer);
                mSendBufferSize = s_minimumSocketBuffer;
            }
        }

        if (GetConfiguration()->receiveBufferSize == 0)
        {
            socket_base::receive_buffer_size option;
            socket.get_option(option);
            SetReceiveBufferSize(option.value());

            if (GetConfiguration()->receiveBufferSize < s_minimumSocketBuffer)
            {
                SetReceiveBufferSize(s_minimumSocketBuffer);
                mReceiveBufferSize = s_minimumSocketBuffer;
            }
        }
    }

    if (GetConfiguration()->maxMessageSize > s_maximumMessageSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than 65000");
        return false;
    }

    if (GetConfiguration()->maxMessageSize > GetConfiguration()->sendBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than sendBufferSize");
        return false;
    }

    if (GetConfiguration()->maxMessageSize > GetConfiguration()->receiveBufferSize)
    {
        logError(RTPS_MSG_OUT, "maxMessageSize cannot be greater than receiveBufferSize");
        return false;
    }

    // TODO(Ricardo) Create an event that update this list.
    GetIPs(currentInterfaces);

    return true;
}

bool UDPTransportInterface::IsInputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(IPLocator::getPhysicalPort(locator)) != mInputSockets.end());
}

bool UDPTransportInterface::IsLocatorSupported(const Locator_t& locator) const
{
    return locator.kind == mTransportKind;
}

bool UDPTransportInterface::IsOutputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsLocatorSupported(locator))
        return false;

    return mOutputSockets.size() > 0;
}

bool UDPTransportInterface::OpenAndBindInputSockets(const Locator_t& locator, TransportReceiverInterface* receiver,
    bool is_multicast, uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    try
    {
        std::vector<std::string> vInterfaces = GetBindingInterfacesList();
        for (std::string sInterface : vInterfaces)
        {
            UDPChannelResource* pChannelResource;
            pChannelResource = CreateInputChannelResource(sInterface, locator, is_multicast, maxMsgSize, receiver);
            mInputSockets[IPLocator::getPhysicalPort(locator)].push_back(pChannelResource);
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")"
            << " with msg: " << e.what());
        mInputSockets.erase(IPLocator::getPhysicalPort(locator));
        return false;
    }

    return true;
}

UDPChannelResource* UDPTransportInterface::CreateInputChannelResource(const std::string& sInterface, const Locator_t& locator,
    bool is_multicast, uint32_t maxMsgSize, TransportReceiverInterface* receiver)
{
    eProsimaUDPSocket unicastSocket = OpenAndBindInputSocket(sInterface, IPLocator::getPhysicalPort(locator), is_multicast);
    UDPChannelResource* pChannelResource = new UDPChannelResource(unicastSocket, maxMsgSize);
    pChannelResource->SetMessageReceiver(receiver);
    pChannelResource->SetInterface(sInterface);
    std::thread* newThread = new std::thread(&UDPTransportInterface::performListenOperation, this,
        pChannelResource, locator);
    pChannelResource->SetThread(newThread);
    return pChannelResource;
}

bool UDPTransportInterface::OpenAndBindOutputSockets(const Locator_t& locator)
{
    (void)locator;

    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    try
    {
        uint16_t port = GetConfiguration()->m_output_udp_socket;
        std::vector<IPFinder::info_IP> locNames;
        GetIPs(locNames);
        // If there is no whitelist, we can simply open a generic output socket
        // and gain efficiency.
        if (IsInterfaceWhiteListEmpty())
        {
            eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(GenerateAnyAddressEndpoint(port), port);
            getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback(true));

            // Outbounding first interface with already created socket.
            if(!locNames.empty())
            {
                SetSocketOutboundInterface(unicastSocket, (*locNames.begin()).name);
            }

            // If more than one interface, then create sockets for outbounding multicast.
            if (locNames.size() > 1)
            {
                auto locIt = locNames.begin();
                mOutputSockets.push_back(new UDPChannelResource(unicastSocket));

                // Create other socket for outbounding rest of interfaces.
                for (++locIt; locIt != locNames.end(); ++locIt)
                {
                    uint16_t new_port = 0;
                    eProsimaUDPSocket multicastSocket = OpenAndBindUnicastOutputSocket(GenerateEndpoint((*locIt).name, new_port), new_port);
                    SetSocketOutboundInterface(multicastSocket, (*locIt).name);

                    UDPChannelResource* mSocket = new UDPChannelResource(multicastSocket);
                    mSocket->only_multicast_purpose(true);
                    mOutputSockets.push_back(mSocket);
                }
            }
            else
            {
                // Multicast data will be sent for the only one interface.
                UDPChannelResource *mSocket = new UDPChannelResource(unicastSocket);
                mOutputSockets.push_back(mSocket);
            }
        }
        else
        {
            locNames.clear();
            GetIPs(locNames, true);

            bool firstInterface = false;
            for (const auto& infoIP : locNames)
            {
                if (IsInterfaceAllowed(infoIP.name))
                {
                    eProsimaUDPSocket unicastSocket = OpenAndBindUnicastOutputSocket(GenerateEndpoint(infoIP.name, port), port);
                    SetSocketOutboundInterface(unicastSocket, infoIP.name);
                    if (!firstInterface)
                    {
                        getSocketPtr(unicastSocket)->set_option(ip::multicast::enable_loopback(true));
                        firstInterface = true;
                    }
                    mOutputSockets.push_back(new UDPChannelResource(unicastSocket));
                }
            }
        }
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPTransport Error binding at port: (" << IPLocator::getPhysicalPort(locator) << ")" << " with msg: " << e.what());
        for (auto& socket : mOutputSockets)
        {
            delete socket;
        }
        mOutputSockets.clear();
        return false;
    }

    return true;
}

eProsimaUDPSocket UDPTransportInterface::OpenAndBindUnicastOutputSocket(const ip::udp::endpoint& endpoint, uint16_t& port)
{
    eProsimaUDPSocket socket = createUDPSocket(mService);
    getSocketPtr(socket)->open(GenerateProtocol());
    if (mSendBufferSize != 0)
    {
        getSocketPtr(socket)->set_option(socket_base::send_buffer_size(mSendBufferSize));
    }
    getSocketPtr(socket)->set_option(ip::multicast::hops(GetConfiguration()->TTL));
    getSocketPtr(socket)->bind(endpoint);

    if (port == 0)
    {
        port = getSocketPtr(socket)->local_endpoint().port();
    }

    return socket;
}

bool UDPTransportInterface::OpenOutputChannel(const Locator_t& locator)
{
    if (!IsLocatorSupported(locator) || IsOutputChannelOpen(locator))
        return false;

    return OpenAndBindOutputSockets(locator);
}

bool UDPTransportInterface::OpenExtraOutputChannel(const Locator_t&)
{
    return false;
}

void UDPTransportInterface::performListenOperation(UDPChannelResource* pChannelResource, Locator_t input_locator)
{
    Locator_t remoteLocator;

    while (pChannelResource->IsAlive())
    {
        // Blocking receive.
        auto& msg = pChannelResource->GetMessageBuffer();
        if (!Receive(pChannelResource, msg.buffer, msg.max_size, msg.length, remoteLocator))
        {
            continue;
        }

        // Processes the data through the CDR Message interface.
        auto receiver = pChannelResource->GetMessageReceiver();
        if (receiver != nullptr)
        {
            receiver->OnDataReceived(msg.buffer, msg.length, input_locator, remoteLocator);
        }
        else
        {
            logWarning(RTPS_MSG_IN, "Received Message, but no receiver attached");
        }
    }
}

bool UDPTransportInterface::Receive(UDPChannelResource* pChannelResource, octet* receiveBuffer,
    uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize, Locator_t& remoteLocator)
{
    try
    {
        ip::udp::endpoint senderEndpoint;

        size_t bytes = pChannelResource->getSocket()->receive_from(asio::buffer(receiveBuffer, receiveBufferCapacity), senderEndpoint);
        receiveBufferSize = static_cast<uint32_t>(bytes);
        if (receiveBufferSize > 0)
        {
            if (receiveBufferSize == 13 && memcmp(receiveBuffer, "EPRORTPSCLOSE", 13) == 0)
            {
                return false;
            }
            EndpointToLocator(senderEndpoint, remoteLocator);
        }
        return (receiveBufferSize > 0);
    }
    catch (const std::exception& error)
    {
        (void)error;
        logWarning(RTPS_MSG_OUT, "Error receiving data: " << error.what());
        return false;
    }
}

bool UDPTransportInterface::ReleaseInputChannel(const Locator_t& locator, const asio::ip::address& interface_address)
{
    try
    {
        uint16_t port = IPLocator::getPhysicalPort(locator);

        if(IsInterfaceWhiteListEmpty())
        {
            Locator_t localLocator;
            FillLocalIp(localLocator);

            ip::udp::socket socket(mService);
            socket.open(GenerateProtocol());
            socket.bind(GenerateLocalEndpoint(localLocator, 0));

            // We first send directly to localhost, in case all network interfaces are disabled
            // (which would mean that multicast traffic may not be sent)
            auto localEndpoint = GenerateLocalEndpoint(localLocator, port);
            socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), localEndpoint);

            // We then send to the address of the input locator
            auto destinationEndpoint = GenerateLocalEndpoint(locator, port);

            asio::error_code ec;
            socket_base::message_flags flags = 0;

            // We ignore the error message because some OS don't allow this functionality like Windows (WSAENETUNREACH) or Mac (EADDRNOTAVAIL)
            socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), destinationEndpoint,flags, ec);
            
            socket.close();
        }
        else
        {
            ip::udp::socket socket(mService);
            socket.open(GenerateProtocol());
            socket.bind(asio::ip::udp::endpoint(interface_address, 0));

            auto localEndpoint = ip::udp::endpoint(interface_address, port);
            socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), localEndpoint);

            // We then send to the address of the input locator
            auto destinationEndpoint = GenerateLocalEndpoint(locator, port);
            socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), destinationEndpoint);

            socket.close();
        }
    }
    catch (const std::exception& error)
    {
        logError(RTPS_MSG_OUT, "Error: " << error.what());
        return false;
    }

    return true;
}

Locator_t UDPTransportInterface::RemoteToMainLocal(const Locator_t& remote) const
{
    if (!IsLocatorSupported(remote))
        return false;

    Locator_t mainLocal(remote);
    //memset(mainLocal.address, 0x00, sizeof(mainLocal.address));
    mainLocal.set_Invalid_Address();
    return mainLocal;
}

bool UDPTransportInterface::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator, const Locator_t& remoteLocator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mOutputMapMutex);
    if (!IsOutputChannelOpen(localLocator) || sendBufferSize > GetConfiguration()->sendBufferSize)
        return false;

    bool success = false;
    bool is_multicast_remote_address = IPLocator::isMulticast(remoteLocator);

    for (auto& socket : mOutputSockets)
    {
        if (is_multicast_remote_address || !socket->only_multicast_purpose())
            success |= SendThroughSocket(sendBuffer, sendBufferSize, remoteLocator, getRefFromPtr(socket->getSocket()));
    }

    return success;
}

bool UDPTransportInterface::Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& /*localLocator*/, const Locator_t& remoteLocator, ChannelResource *pChannelResource)
{
    UDPChannelResource *udpSocket = dynamic_cast<UDPChannelResource*>(pChannelResource);
    return SendThroughSocket(sendBuffer, sendBufferSize, remoteLocator, getRefFromPtr(udpSocket->getSocket()));
}

bool UDPTransportInterface::SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize,
    const Locator_t& remoteLocator, eProsimaUDPSocketRef socket)
{
    auto destinationEndpoint = GenerateEndpoint(remoteLocator, IPLocator::getPhysicalPort(remoteLocator));

    size_t bytesSent = 0;

    try
    {
        bytesSent = getSocketPtr(socket)->send_to(asio::buffer(sendBuffer, sendBufferSize), destinationEndpoint);
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, error.what());
        return false;
    }

    (void)bytesSent;
    logInfo(RTPS_MSG_OUT, "UDPTransport: " << bytesSent << " bytes TO endpoint: " << destinationEndpoint
        << " FROM " << getSocketPtr(socket)->local_endpoint());
    return true;
}

LocatorList_t UDPTransportInterface::ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists)
{
    LocatorList_t multicastResult, unicastResult;
    std::vector<MultiUniLocatorsLinkage> pendingLocators;

    for (auto& locatorList : locatorLists)
    {
        LocatorListConstIterator it = locatorList.begin();
        bool multicastDefined = false;
        LocatorList_t pendingMulticast, pendingUnicast;

        while (it != locatorList.end())
        {
            assert((*it).kind == mTransportKind);

            if (IPLocator::isMulticast(*it))
            {
                // If the multicast locator is already chosen, not choose any unicast locator.
                if (multicastResult.contains(*it))
                {
                    multicastDefined = true;
                    pendingUnicast.clear();
                }
                else
                {
                    // Search the multicast locator in pending locators.
                    auto pending_it = pendingLocators.begin();
                    bool found = false;

                    while (pending_it != pendingLocators.end())
                    {
                        if ((*pending_it).multicast.contains(*it))
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
                    if (!found)
                        pendingMulticast.push_back(*it);
                }
            }
            else
            {
                if (!multicastDefined)
                {
                    // Check is local interface.
                    auto localInterface = currentInterfaces.begin();
                    for (; localInterface != currentInterfaces.end(); ++localInterface)
                    {
                        if (CompareLocatorIP(localInterface->locator, *it))
                        {
                            // Check 127.0.0.1 in the whitelist
                            Locator_t loopbackLocator;
                            FillLocalIp(loopbackLocator);
                            if (IsLocatorAllowed(loopbackLocator))
                            {
                                // Loopback locator
                                IPLocator::setPhysicalPort(loopbackLocator, IPLocator::getPhysicalPort(*it));
                                pendingUnicast.push_back(loopbackLocator);
                                break;
                            }
                            else
                            {
                                // Check interface in whitelist
                                if (IsLocatorAllowed(*it))
                                {
                                    // Custom Loopback locator
                                    pendingUnicast.push_back(*it);
                                    break;
                                }
                            }
                        }
                    }

                    if (localInterface == currentInterfaces.end())
                    {
                        pendingUnicast.push_back(*it);
                    }
                }
            }

            ++it;
        }

        if (pendingMulticast.size() == 0)
        {
            unicastResult.push_back(pendingUnicast);
        }
        else if (pendingUnicast.size() == 0)
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
    for (auto link : pendingLocators)
        result.push_back(link.unicast);

    return result;
}

bool UDPTransportInterface::fillMetatrafficMulticastLocator(Locator_t &locator,
        uint32_t metatraffic_multicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_multicast_port;
    }
    return true;
}

bool UDPTransportInterface::fillMetatrafficUnicastLocator(Locator_t &locator,
        uint32_t metatraffic_unicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_unicast_port;
    }
    return true;
}

bool UDPTransportInterface::configureInitialPeerLocator(Locator_t &locator, const PortParameters &port_params,
        uint32_t domainId, LocatorList_t& list) const
{
    if(locator.port == 0)
    {
        for(uint32_t i = 0; i < GetConfiguration()->maxInitialPeersRange; ++i)
        {
            Locator_t auxloc(locator);
            auxloc.port = port_params.getUnicastPort(domainId, i);

            list.push_back(auxloc);
        }
    }
    else
        list.push_back(locator);

    return true;
}

bool UDPTransportInterface::fillUnicastLocator(Locator_t &locator, uint32_t well_known_port) const
{
    if (locator.port == 0)
    {
        locator.port = well_known_port;
    }
    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
