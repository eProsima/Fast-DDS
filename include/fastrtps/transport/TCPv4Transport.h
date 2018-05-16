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

#include <vector>
#include <memory>
#include <map>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace rtps{

// TCP Header structs and enums.
struct TCPHeader
{
    const char rtcp[4];
    uint32_t length;
    uint32_t crc;
    uint16_t logicalPort;

    TCPHeader() :
		rtcp{'R','T','C','P'}
        , length(sizeof(TCPHeader))
        , crc(0)
        , logicalPort(0)
    {
        //memcpy((char*)rtcp, "RTCP", sizeof(char) * 4);
    }
};

union TCPTransactionId
{
    uint32_t ints[3];
    octet octets[12];

    TCPTransactionId& operator=(const octet* id)
    {
        memcpy(octets, id, 12 * sizeof(octet));
        return *this;
    }

    TCPTransactionId& operator=(const char* id)
    {
        memcpy(octets, id, 12 * sizeof(octet));
        return *this;
    }

    TCPTransactionId& operator=(uint32_t id)
    {
        ints[0] = id;
        ints[1] = 0;
        ints[2] = 0;
        return *this;
    }

    TCPTransactionId& operator=(uint64_t id)
    {
        memset(ints, 0, sizeof(uint32_t) * 3);
        memcpy(ints, &id, sizeof(uint64_t));
        return *this;
    }
};

enum TCPCPMKind : octet
{
    BIND_CONNECTION_REQUEST =           0xD1,
    BIND_CONNECTION_RESPONSE =          0xE1,
    OPEN_LOGICAL_PORT_REQUEST =         0xD2,
    OPEN_LOGICAL_PORT_RESPONSE =        0xE2,
    CHECK_LOGICAL_PORT_REQUEST =        0xD3,
    CHECK_LOGICAL_PORT_RESPONSE =       0xE3,
    KEEP_ALIVE_REQUEST =                0xD4,
    KEEP_ALIVE_RESPONSE =               0xE4,
    LOGICAL_PORT_IS_CLOSED_REQUEST =    0xD5,
    UNBIND_CONNECTION_REQUEST =         0xD6
};

struct TCPControlMsgHeader
{
    TCPCPMKind kind;
    octet flags;
    uint16_t length;
    TCPTransactionId transactionId;

    void setFlags(bool endianess, bool hasPayload, bool requiresResponse)
    {
        octet e = (endianess) ? BIT(1) : 0x00;
        octet p = (hasPayload) ? BIT(2) : 0x00;
        octet r = (requiresResponse) ? BIT(3) : 0x00;
        flags = e | p | r;
    }

    void setEndianess(Endianness_t endianess)
    {
        // Endianess flag has inverse logic than Endianness_t :-/
        if (endianess == Endianness_t::BIGEND)
        {
            flags &= 0xFE;
        }
        else
        {
            flags |= BIT(1);
        }
    }

    void setHasPayload(bool hasPayload)
    {
        if (hasPayload)
        {
            flags |= BIT(2);
        }
        else
        {
            flags &= 0xFD;
        }
    }

    void setRequiresResponse(bool requiresResponse)
    {
        if (requiresResponse)
        {
            flags |= BIT(3);
        }
        else
        {
            flags &= 0xFB;
        }
    }

    bool getEndianess()
    {
        return (flags & BIT(1)) != 0;
    }

    bool getHasPayload()
    {
        return (flags & BIT(2)) != 0;
    }

    bool getRequiresResponse()
    {
        return (flags & BIT(3)) != 0;
    }
};

class TCPv4Transport;

class TCPAcceptor
{
public:
	asio::ip::tcp::acceptor m_acceptor;
	uint16_t m_port;
	uint32_t m_receiveBufferSize;

    TCPAcceptor(asio::io_service& io_service, uint16_t port, uint32_t receiveBufferSize);

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

   /**
    * Starts listening on the specified port, and if the specified address is in the
    * multicast range, it joins the specified multicast group,
    */
   virtual bool OpenInputChannel(const Locator_t&) override;

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

   void SocketAccepted(uint32_t port, uint32_t receiveBufferSize, const asio::error_code& error, asio::ip::tcp::socket s);
   void SocketConnected(uint32_t logical_port, uint32_t sendBufferSize, const asio::error_code& error);
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

   std::map<uint32_t, TCPConnector*> mPendingOutputSockets;     // The Key is the "Logical Port"
   std::map<uint32_t, std::vector<TCPSocketInfo>> mOutputSockets;  // The Key is the "Logical Port"
   std::map<uint32_t, Semaphore*> mOutputSemaphores;            // Control the physical connection

   std::vector<IPFinder::info_IP> currentInterfaces;

   struct LocatorCompare{ bool operator()(const Locator_t& lhs, const Locator_t& rhs) const
                        {return (memcmp(&lhs, &rhs, sizeof(Locator_t)) < 0); } };

   std::map<uint32_t, std::shared_ptr<TCPAcceptor>> mPendingInputSockets;
   std::map<uint32_t, std::vector<eProsimaTCPSocket>> mInputSockets;

   bool IsInterfaceAllowed(const asio::ip::address_v4& ip);
   std::vector<asio::ip::address_v4> mInterfaceWhiteList;

   bool OpenAndBindOutputSockets(Locator_t& locator);
   void OpenAndBindUnicastOutputSocket(const asio::ip::address_v4&, uint16_t logical_port, uint16_t tcp_port);

   bool OpenAndBindInputSockets(uint32_t port);
   bool ReceiveDataCB(const asio::error_code& error, std::size_t bytes_transferred);

   bool SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator,
       eProsimaTCPSocket& socket);

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
