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
#include <fastrtps/rtps/network/ReceiverResource.h>
#include <fastrtps/rtps/network/SenderResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

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
    : TransportDescriptorInterface(s_maximumMessageSize)
    , m_output_udp_socket(0)
{
}

UDPTransportDescriptor::UDPTransportDescriptor(const UDPTransportDescriptor& t)
    : TransportDescriptorInterface(t)
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

void UDPTransportInterface::AssociateSenderToSocket(UDPChannelResource *socket, SenderResource *sender) const
{
    //sender->SetChannelResource(socket);

    auto it = mSocketToSenders.find(socket);
    if (it == mSocketToSenders.end())
    {
        mSocketToSenders[socket].emplace_back(sender);
    }
    else
    {
        auto srit = std::find((*it).second.begin(), (*it).second.end(), sender);
        if (srit == (*it).second.end())
        {
            (*it).second.emplace_back(sender);
        }
        // else already associated
    }
}

void UDPTransportInterface::Clean()
{
    if (ioServiceThread)
    {
        mService.stop();
        ioServiceThread->join();
    }

    assert(mInputSockets.size() == 0);
    assert(mOutputSockets.size() == 0);
}

bool UDPTransportInterface::CloseInputChannel(const Locator_t& locator)
{
    UDPChannelResource* pChannelResource = nullptr;
    {
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
        if (!IsInputChannelOpen(locator))
            return false;

        ReleaseInputChannel(locator);
        pChannelResource = mInputSockets.at(locator.get_physical_port());
        pChannelResource->getSocket()->cancel();
        pChannelResource->getSocket()->close();
        mInputSockets.erase(locator.get_physical_port());
    }

    if (pChannelResource != nullptr)
    {
        delete pChannelResource;
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

    return true;
}

bool UDPTransportInterface::DoInputLocatorsMatch(const Locator_t& left, const Locator_t& right) const
{
    return left.get_port() == right.get_port();
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

    auto ioServiceFunction = [&]()
    {
        io_service::work work(mService);
        mService.run();
    };
    ioServiceThread.reset(new std::thread(ioServiceFunction));

    return true;
}

bool UDPTransportInterface::IsInputChannelOpen(const Locator_t& locator) const
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    return IsLocatorSupported(locator) && (mInputSockets.find(locator.get_physical_port()) != mInputSockets.end());
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

eProsimaUDPSocket UDPTransportInterface::OpenAndBindInputSocket(uint16_t port, bool is_multicast)
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
    }

    getSocketPtr(socket)->bind(GenerateAnyAddressEndpoint(port));

    return socket;
}

bool UDPTransportInterface::OpenAndBindInputSockets(const Locator_t& locator, ReceiverResource* receiverResource,
    bool is_multicast, uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    try
    {
        eProsimaUDPSocket unicastSocket = OpenAndBindInputSocket(locator.get_physical_port(), is_multicast);
        UDPChannelResource* pChannelResource = new UDPChannelResource(unicastSocket, maxMsgSize);
        pChannelResource->SetMessageReceiver(receiverResource->CreateMessageReceiver());
        std::thread* newThread = new std::thread(&UDPTransportInterface::performListenOperation, this,
            pChannelResource, locator);
        pChannelResource->SetThread(newThread);
        mInputSockets.emplace(locator.get_physical_port(), pChannelResource);
    }
    catch (asio::system_error const& e)
    {
        (void)e;
        logInfo(RTPS_MSG_OUT, "UDPTransport Error binding at port: (" << locator.get_physical_port() << ")"
            << " with msg: " << e.what());
        mInputSockets.erase(locator.get_physical_port());
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

bool UDPTransportInterface::OpenOutputChannel(const Locator_t& locator, SenderResource* senderResource, uint32_t /*msgSize*/)
{
    if (!IsLocatorSupported(locator) || IsOutputChannelOpen(locator))
        return false;

    return OpenAndBindOutputSockets(locator, senderResource);
}

bool UDPTransportInterface::OpenExtraOutputChannel(const Locator_t&, SenderResource*, uint32_t size)
{
    (void)size;
    return false;
};

void UDPTransportInterface::performListenOperation(UDPChannelResource* pChannelResource, Locator_t input_locator)
{
    Locator_t remoteLocator;
    while (pChannelResource->IsAlive())
    {
        // Blocking receive.
        auto msg = pChannelResource->GetMessageBuffer();
        CDRMessage::initCDRMsg(&msg);
        if (!Receive(msg.buffer, msg.max_size, msg.length, pChannelResource, input_locator, remoteLocator))
            continue;

        // Processes the data through the CDR Message interface.
        MessageReceiver* receiver = pChannelResource->GetMessageReceiver();
        if (receiver != nullptr)
        {
            receiver->processCDRMsg(rtpsParticipantGuidPrefix, &input_locator, &msg);
        }
        else
        {
            logWarning(RTCP, "Received Message, but no MessageReceiver attached");
        }
    }
}

bool UDPTransportInterface::Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
    ChannelResource* pChannelResource, const Locator_t& inputLocator, Locator_t& remoteLocator)
{
    if (!pChannelResource->IsAlive())
    {
        return false;
    }

    ip::udp::endpoint senderEndpoint;
    UDPChannelResource* socket = nullptr;

    { // lock scope
        std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
        if (!pChannelResource->IsAlive())
        {
            return false;
        }

        auto socketIt = mInputSockets.find(static_cast<uint16_t>(inputLocator.get_port()));
        if (socketIt != mInputSockets.end())
        {
            socket = socketIt->second;
        }
    }

    try
    {
        if (socket != nullptr)
        {
            size_t bytes = socket->getSocket()->receive_from(asio::buffer(receiveBuffer, receiveBufferCapacity), senderEndpoint);
            receiveBufferSize = static_cast<uint32_t>(bytes);
            if (receiveBufferSize > 0)
            {
                if (receiveBufferSize == 13 && memcmp(receiveBuffer, "EPRORTPSCLOSE", 13) == 0)
                {
                    return false;
                }
                EndpointToLocator(senderEndpoint, remoteLocator);
            }
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

bool UDPTransportInterface::ReleaseInputChannel(const Locator_t& locator)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);
    if (!IsInputChannelOpen(locator))
    {
        return false;
    }

    try
    {
        ip::udp::socket socket(mService);
        socket.open(GenerateProtocol());
        auto destinationEndpoint = GenerateLocalEndpoint(locator, static_cast<uint16_t>(locator.get_port()));
        socket.send_to(asio::buffer("EPRORTPSCLOSE", 13), destinationEndpoint);
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
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
    bool is_multicast_remote_address = remoteLocator.is_Multicast();

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
    auto destinationEndpoint = GenerateEndpoint(remoteLocator, remoteLocator.get_physical_port());

    size_t bytesSent = 0;

    try
    {
        bytesSent = getSocketPtr(socket)->send_to(asio::buffer(sendBuffer, sendBufferSize), destinationEndpoint);
    }
    catch (const std::exception& error)
    {
        logWarning(RTPS_MSG_OUT, "Error: " << error.what());
        return false;
    }

    (void)bytesSent;
    logInfo(RTPS_MSG_OUT, "UDPTransport: " << bytesSent << " bytes TO endpoint: " << destinationEndpoint
        << " FROM " << getSocketPtr(socket)->local_endpoint());
    return true;
}
void UDPTransportInterface::SetParticipantGUIDPrefix(const GuidPrefix_t& prefix)
{
    rtpsParticipantGuidPrefix = prefix;
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

            if (it->is_Multicast())
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
                            // Loopback locator
                            Locator_t loopbackLocator;
                            FillLocalIp(loopbackLocator);
                            loopbackLocator.set_port(it->get_physical_port());
                            pendingUnicast.push_back(loopbackLocator);
                            break;
                        }
                    }

                    if (localInterface == currentInterfaces.end())
                        pendingUnicast.push_back(*it);
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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
