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

#ifndef UDPV6_TRANSPORT_H
#define UDPV6_TRANSPORT_H

#include <asio.hpp>
#include <thread>

#include "TransportInterface.h"
#include "SocketInfo.h"
#include "UDPv6TransportDescriptor.h"
#include "../utils/IPFinder.h"

#include <vector>
#include <memory>
#include <map>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * This is a default UDPv6 implementation.
 *    - Opening an output channel by passing a locator will open a socket per interface on the given port.
 *       This collection of sockets constitute the "outbound channel". In other words, a channel corresponds
 *       to a port + a direction.
 *
 *    - It is possible to provide a white list at construction, which limits the interfaces the transport
 *       will ever be able to interact with. If left empty, all interfaces are allowed.
 *
 *    - Opening an input channel by passing a locator will open a socket listening on the given port on every
 *       whitelisted interface, and join the multicast channel specified by the locator address. Hence, any locator
 *       that does not correspond to the multicast range will simply open the port without a subsequent join. Joining
 *       multicast groups late is supported by attempting to open the channel again with the same port + a
 *       multicast address (the OpenInputChannel function will fail, however, because no new channel has been
 *       opened in a strict sense).
 * @ingroup TRANSPORT_MODULE
 */

class UDPv6Transport : public TransportInterface
{
public:

   RTPS_DllAPI UDPv6Transport(const UDPv6TransportDescriptor&);

   virtual ~UDPv6Transport() override;

   bool init() override;

   //! Checks whether there are open and bound sockets for the given port.
   virtual bool IsInputChannelOpen(const Locator_t&) const override;

   /**
    * Checks whether there are open and bound sockets for the given port.
    */
   virtual bool IsOutputChannelOpen(const Locator_t&, SenderResource* senderResource = nullptr) const override;

   //! Checks for UDPv6 kind.
   virtual bool IsLocatorSupported(const Locator_t&) const override;

   //! Reports whether Locators correspond to the same port.
   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const override;

   /**
    * Converts a given remote locator (that is, a locator referring to a remote
    * destination) to the main local locator whose channel can write to that
    * destination. In this case it will return a IP_ANY address on that port.
    */
   virtual Locator_t RemoteToMainLocal(const Locator_t&) const override;

   //! Sets the ID of the participant that has created the transport.
   virtual void SetParticipantGUIDPrefix(const GuidPrefix_t& prefix) override;

   /**
    * Starts listening on the specified port, and if the specified address is in the
    * multicast range, it joins the specified multicast group,
    */
   virtual bool OpenInputChannel(const Locator_t&, ReceiverResource*, uint32_t) override;

   /**
    * Opens a socket on the given address and port (as long as they are white listed).
    */
   virtual bool OpenOutputChannel(const Locator_t&, SenderResource*) override;
   virtual bool OpenExtraOutputChannel(Locator_t&, SenderResource*) override { return false; };

   //! Removes the listening socket for the specified port.
   virtual bool CloseInputChannel(const Locator_t&) override;

   //! Release the listening socket for the specified port.
   virtual bool ReleaseInputChannel(const Locator_t&) override;

   //! Removes all outbound sockets on the given port.
   virtual bool CloseOutputChannel(const Locator_t&) override;

   /**
    * Blocking Send through the specified channel. In both modes, using a localLocator of ANY will
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
                        const Locator_t& remoteLocator, SocketInfo* pSocketInfo) override;
   /**
    * Blocking Receive from the specified channel.
    * @param receiveBuffer vector with enough capacity (not size) to accomodate a full receive buffer. That
    * capacity must not be less than the receiveBufferSize supplied to this class during construction.
    * @param localLocator Locator mapping to the local channel we're listening to.
    * @param[out] remoteLocator Locator describing the remote restination we received a packet from.
    */
   bool Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
       SocketInfo* socketInfo, Locator_t& remoteLocator);

   virtual LocatorList_t NormalizeLocator(const Locator_t& locator) override;

   virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;

   virtual bool is_local_locator(const Locator_t& locator) const override;

   TransportDescriptorInterface* get_configuration() override { return &mConfiguration_; }

   virtual void AddDefaultLocator(LocatorList_t &defaultList) override;

private:

   UDPv6TransportDescriptor mConfiguration_;
   uint32_t mSendBufferSize;
   uint32_t mReceiveBufferSize;

   // For UDPv6, the notion of channel corresponds to a port + direction tuple.
	asio::io_service mService;
   std::unique_ptr<std::thread> ioServiceThread;

   mutable std::recursive_mutex mOutputMapMutex;
   mutable std::recursive_mutex mInputMapMutex;

   //! The notion of output channel corresponds to a port.
   std::map<uint16_t, std::vector<UDPSocketInfo*> > mOutputSockets;

   std::vector<IPFinder::info_IP> currentInterfaces;

   //! The notion of output channel corresponds to an address.
   struct LocatorCompare{ bool operator()(const Locator_t& lhs, const Locator_t& rhs) const
                        {return (memcmp(&lhs, &rhs, sizeof(Locator_t)) < 0); } };
   //! For both modes, an input channel corresponds to a port.
   std::map<uint16_t, UDPSocketInfo*> mInputSockets;

   bool IsInterfaceAllowed(const asio::ip::address_v6& ip);
   std::vector<asio::ip::address_v6> mInterfaceWhiteList;


   bool OpenAndBindOutputSockets(const Locator_t& locator, SenderResource*);
   bool OpenAndBindInputSockets(const Locator_t& locator, ReceiverResource* receiverResource, bool is_multicast,
       uint32_t maxMsgSize);

   eProsimaUDPSocket OpenAndBindUnicastOutputSocket(const asio::ip::address_v6&, uint16_t& port);
   eProsimaUDPSocket OpenAndBindInputSocket(uint16_t port, bool is_multicast);

   void performListenOperation(UDPSocketInfo* pSocketInfo, Locator_t input_locator);

   bool SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator,
       eProsimaUDPSocketRef socket);

    mutable std::map<UDPSocketInfo*, std::vector<SenderResource*>> mSocketToSenders;
    void AssociateSenderToSocket(UDPSocketInfo*, SenderResource*) const;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
