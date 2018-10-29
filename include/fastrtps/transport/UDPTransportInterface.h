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

#ifndef UDP_TRANSPORT_INTERFACE_H
#define UDP_TRANSPORT_INTERFACE_H

#include <asio.hpp>
#include <thread>

#include "TransportInterface.h"
#include "UDPChannelResource.h"
#include "UDPTransportDescriptor.h"
#include "../utils/IPFinder.h"

#include <vector>
#include <memory>
#include <map>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class UDPTransportInterface : public TransportInterface
{
public:

   virtual ~UDPTransportInterface() override;

   void Clean();

   //! Removes the listening socket for the specified port.
   virtual bool CloseInputChannel(const Locator_t&) override;

   //! Removes all outbound sockets on the given port.
   virtual bool CloseOutputChannel(const Locator_t&) override;

   //! Reports whether Locators correspond to the same port.
   virtual bool DoInputLocatorsMatch(const Locator_t&, const Locator_t&) const override;
   virtual bool DoOutputLocatorsMatch(const Locator_t&, const Locator_t&) const override;

   virtual const UDPTransportDescriptor* GetConfiguration() const = 0;

   bool init() override;

   //! Checks whether there are open and bound sockets for the given port.
   virtual bool IsInputChannelOpen(const Locator_t&) const override;

   //! Checks for TCP kinds.
   virtual bool IsLocatorSupported(const Locator_t&) const override;

   //! Checks whether there are open and bound sockets for the given port.
   virtual bool IsOutputChannelOpen(const Locator_t&) const override;

   //! Opens a socket on the given address and port (as long as they are white listed).
   virtual bool OpenOutputChannel(const Locator_t&) override;
   virtual bool OpenExtraOutputChannel(const Locator_t&) override;

   /**
   * Blocking Receive from the specified channel.
   * @param pChannelResource Pointer to the channer resource that stores the socket.
   * @param receiveBuffer vector with enough capacity (not size) to accomodate a full receive buffer. That
   * capacity must not be less than the receiveBufferSize supplied to this class during construction.
   * @param receiveBufferCapacity Maximum size of the receiveBuffer.
   * @param[out] receiveBufferSize Size of the received buffer.
   * @param[out] remoteLocator Locator describing the remote restination we received a packet from.
   */
   bool Receive(UDPChannelResource* pChannelResource, octet* receiveBuffer,
       uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize, Locator_t& remoteLocator);

   //! Release the listening socket for the specified port.
   virtual bool ReleaseInputChannel(const Locator_t&) override;

   /**
   * Converts a given remote locator (that is, a locator referring to a remote
   * destination) to the main local locator whose channel can write to that
   * destination. In this case it will return a 0.0.0.0 address on that port.
   */
   virtual Locator_t RemoteToMainLocal(const Locator_t&) const override;

   /**
   * Blocking Send through the specified channel. In both modes, using a localLocator of 0.0.0.0 will
   * send through all whitelisted interfaces provided the channel is open.
   * @param sendBuffer Slice into the raw data to send.
   * @param sendBufferSize Size of the raw data. It will be used as a bounds check for the previous argument.
   * It must not exceed the sendBufferSize fed to this class during construction.
   * @param localLocator Locator mapping to the channel we're sending from.
   * @param remoteLocator Locator describing the remote destination we're sending to.
   */
   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator,
       const Locator_t& remoteLocator) override;

   virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator,
       const Locator_t& remoteLocator, ChannelResource* pChannelResource) override;

   virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;

    virtual bool fillMetatrafficMulticastLocator(Locator_t &locator,
        uint32_t metatraffic_multicast_port) const override;

    virtual bool fillMetatrafficUnicastLocator(Locator_t &locator, uint32_t metatraffic_unicast_port) const override;

    virtual bool configureInitialPeerLocator(Locator_t &locator, const PortParameters &port_params, uint32_t domainId,
        LocatorList_t& list) const override;

    virtual bool fillUnicastLocator(Locator_t &locator, uint32_t well_known_port) const override;

protected:

    int32_t mTransportKind;

    // For UDPv6, the notion of channel corresponds to a port + direction tuple.
    asio::io_service mService;
    std::unique_ptr<std::thread> ioServiceThread;
    std::vector<IPFinder::info_IP> currentInterfaces;

    mutable std::recursive_mutex mOutputMapMutex;
    mutable std::recursive_mutex mInputMapMutex;
    std::map<uint16_t, std::vector<UDPChannelResource*>> mInputSockets;
    std::vector<UDPChannelResource*> mOutputSockets;

    uint32_t mSendBufferSize;
    uint32_t mReceiveBufferSize;

    UDPTransportInterface();

    virtual bool CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const = 0;
    virtual bool CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const = 0;

    virtual void EndpointToLocator(asio::ip::udp::endpoint& endpoint, Locator_t& locator) = 0;
    virtual void FillLocalIp(Locator_t& loc) = 0;

    virtual asio::ip::udp::endpoint GenerateAnyAddressEndpoint(uint16_t port) = 0;
    virtual asio::ip::udp::endpoint GenerateEndpoint(uint16_t port) = 0;
    virtual asio::ip::udp::endpoint GenerateEndpoint(const std::string& sIp, uint16_t port) = 0;
    virtual asio::ip::udp::endpoint GenerateEndpoint(const Locator_t& loc, uint16_t port) = 0;
    virtual asio::ip::udp::endpoint GenerateLocalEndpoint(const Locator_t& loc, uint16_t port) = 0;
    virtual asio::ip::udp GenerateProtocol() const = 0;
    virtual void GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false) = 0;
    virtual bool IsInterfaceWhiteListEmpty() const = 0;
    virtual bool IsInterfaceAllowed(const std::string& interface) const = 0;
    virtual std::vector<std::string> GetInterfacesList(const Locator_t& locator) = 0;

    bool OpenAndBindInputSockets(const Locator_t& locator, TransportReceiverInterface* receiver, bool is_multicast,
        uint32_t maxMsgSize);
    virtual eProsimaUDPSocket OpenAndBindInputSocket(const std::string& sIp, uint16_t port, bool is_multicast) = 0;
    bool OpenAndBindOutputSockets(const Locator_t& locator);
    eProsimaUDPSocket OpenAndBindUnicastOutputSocket(const asio::ip::udp::endpoint& endpoint, uint16_t& port);
    /** Function to be called from a new thread, which takes cares of performing a blocking receive
    operation on the ReceiveResource
    @param input_locator - Locator that triggered the creation of the resource
    */
    void performListenOperation(UDPChannelResource* pChannelResource, Locator_t input_locator);

    bool SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator,
        eProsimaUDPSocketRef socket);

    virtual void SetReceiveBufferSize(uint32_t size) = 0;
    virtual void SetSendBufferSize(uint32_t size) = 0;
    virtual void SetSocketOutbountInterface(eProsimaUDPSocket&, const std::string&) = 0;
    /*
        struct LocatorCompare {
        bool operator()(const Locator_t& lhs, const Locator_t& rhs) const
        {
            return (memcmp(&lhs, &rhs, sizeof(Locator_t)) < 0);
        }
    };
    */
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // UDP_TRANSPORT_INTERFACE_H
