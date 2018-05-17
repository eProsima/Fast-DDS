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

#ifndef TCPV4_TRANSPORT_H
#define TCPV4_TRANSPORT_H

#include <asio.hpp>
#include <thread>

#include <fastrtps/utils/Semaphore.h>
#include "TransportInterface.h"
#include "TCPv4TransportDescriptor.h"
#include "SocketInfo.h"
#include "../utils/IPFinder.h"
#include "tcp/RTCPHeader.h"

#include <vector>
#include <memory>
#include <map>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace rtps{
class TCPv4Transport;

class TCPAcceptor
{
public:
	asio::ip::tcp::acceptor m_acceptor;
    Locator_t m_locator;
    std::shared_ptr<MessageReceiver> m_msgReceiver;
	uint32_t m_receiveBufferSize;

    TCPAcceptor(asio::io_service& io_service, const Locator_t& locator, std::shared_ptr<MessageReceiver> msgReceiver,
        uint32_t receiveBufferSize);

	void Accept(TCPv4Transport* parent);
};

class TCPConnector
{
public:
    uint16_t m_logical_port;
    uint16_t m_tcp_port;
	const asio::ip::address_v4 m_ipAddress;
	uint32_t m_sendBufferSize;
	eProsimaTCPSocket m_socket;

	TCPConnector(asio::io_service& io_service, const asio::ip::address_v4& ipAddress, uint16_t tcp_port,
        uint16_t logical_port, uint32_t sendBufferSize);

	void Connect(TCPv4Transport* parent);
	void RetryConnect(asio::io_service& io_service, TCPv4Transport* parent);
};


/**
 * This is a default TCPv4 implementation.
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
class TCPv4Transport : public TransportInterface
{
public:

   RTPS_DllAPI TCPv4Transport(const TCPv4TransportDescriptor&);

   virtual ~TCPv4Transport() override;

   bool init() override;

   //! Checks whether there are open and bound sockets for the given port.
   virtual bool IsInputChannelOpen(const Locator_t&) const override;

   /**
    * Checks whether there are open and bound sockets for the given port.
    */
   virtual bool IsOutputChannelOpen(const Locator_t&) const override;

   //! Checks for TCPv4 kind.
   virtual bool IsLocatorSupported(const Locator_t&) const override;

   //! Reports whether Locators correspond to the same port.
   virtual bool DoLocatorsMatch(const Locator_t&, const Locator_t&) const override;

   /**
    * Converts a given remote locator (that is, a locator referring to a remote
    * destination) to the main local locator whose channel can write to that
    * destination. In this case it will return a 0.0.0.0 address on that port.
    */
   virtual Locator_t RemoteToMainLocal(const Locator_t&) const override;

   //! Sets the ID of the participant that has created the transport.
   virtual void SetParticipantGUIDPrefix(const GuidPrefix_t& prefix) override;

   /**
    * Starts listening on the specified port, and if the specified address is in the
    * multicast range, it joins the specified multicast group,
    */
   virtual bool OpenInputChannel(const Locator_t&, std::shared_ptr<MessageReceiver>) override;

   /**
    * Opens a socket on the given address and port (as long as they are white listed).
    */
   virtual bool OpenOutputChannel(Locator_t&) override;

   //! Removes the listening socket for the specified port.
   virtual bool CloseInputChannel(const Locator_t&) override;

   //! Removes all outbound sockets on the given port.
   virtual bool CloseOutputChannel(const Locator_t&) override;

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
   /**
    * Blocking Receive from the specified channel.
    * @param receiveBuffer vector with enough capacity (not size) to accomodate a full receive buffer. That
    * capacity must not be less than the receiveBufferSize supplied to this class during construction.
    * @param localLocator Locator mapping to the local channel we're listening to.
    * @param[out] remoteLocator Locator describing the remote restination we received a packet from.
    */
   virtual bool Receive(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t& receiveBufferSize,
                        const Locator_t& localLocator, Locator_t& remoteLocator) override;

   virtual LocatorList_t NormalizeLocator(const Locator_t& locator) override;

   virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;

   virtual bool is_local_locator(const Locator_t& locator) const override;

   TransportDescriptorInterface* get_configuration() override { return &mConfiguration_; }

   void SocketAccepted(const Locator_t& locator, std::shared_ptr<MessageReceiver> msgReceiver,
       uint32_t receiveBufferSize, const asio::error_code& error, asio::ip::tcp::socket s);
   void SocketConnected(uint16_t logical_port, uint32_t sendBufferSize, const asio::error_code& error);
protected:

   //! Constructor with no descriptor is necessary for implementations derived from this class.
   TCPv4Transport();
   TCPv4TransportDescriptor mConfiguration_;
   uint32_t mSendBufferSize;
   uint32_t mReceiveBufferSize;

   asio::io_service mService;
   std::unique_ptr<std::thread> ioServiceThread;

   mutable std::recursive_mutex mOutputMapMutex;
   mutable std::recursive_mutex mInputMapMutex;

   std::map<uint16_t, TCPConnector*> mPendingOutputSockets;     // The Key is the "Logical Port"
   std::map<uint16_t, std::vector<TCPSocketInfo>> mOutputSockets;  // The Key is the "Logical Port"
   std::map<uint16_t, Semaphore*> mOutputSemaphores;            // Control the physical connection

   std::vector<IPFinder::info_IP> currentInterfaces;

   struct LocatorCompare{ bool operator()(const Locator_t& lhs, const Locator_t& rhs) const
                        {return (memcmp(&lhs, &rhs, sizeof(Locator_t)) < 0); } };

   std::map<uint16_t, std::shared_ptr<TCPAcceptor>> mPendingInputSockets;
   std::map<uint16_t, std::vector<TCPSocketInfo*>> mInputSockets;

   bool IsInterfaceAllowed(const asio::ip::address_v4& ip);
   std::vector<asio::ip::address_v4> mInterfaceWhiteList;

   bool OpenAndBindOutputSockets(Locator_t& locator);
   void OpenAndBindUnicastOutputSocket(const asio::ip::address_v4&, uint16_t logical_port, uint16_t tcp_port);

   bool OpenAndBindInputSockets(const Locator_t& locator, std::shared_ptr<MessageReceiver> msgReceiver);

   /** Function to be called from a new thread, which takes cares of performing a blocking receive
   operation on the ReceiveResource
   @param input_locator - Locator that triggered the creation of the resource
   */
   void performListenOperation(TCPSocketInfo* pSocketInfo, Locator_t input_locator);

   bool SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator,
       TCPSocketInfo& socket);

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
