// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TCPV6_TRANSPORT_H
#define TCPV6_TRANSPORT_H

#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/transport/tcp/RTCPHeader.h>

#include <asio.hpp>
#include <thread>
#include <vector>
#include <map>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class TCPv6Transport;
class RTCPMessageManager;
class CleanTCPSocketsEvent;
class TCPChannelResource;

class TCPv6Acceptor
{
public:
	asio::ip::tcp::acceptor mAcceptor;
    Locator_t mLocator;
    uint32_t mMaxMsgSize;
#ifndef ASIO_HAS_MOVE
    std::shared_ptr<asio::ip::tcp::socket> mSocket;
    asio::ip::tcp::endpoint mEndPoint;
#endif

    TCPv6Acceptor(asio::io_service& io_service, const Locator_t& locator, uint32_t maxMsgSize);
    ~TCPv6Acceptor()    {    }

    //! Method to start the accepting process.
    void Accept(TCPv6Transport* parent, asio::io_service&);
};

class TCPv6Connector
{
public:
    Locator_t m_locator;
	eProsimaTCPSocket m_socket;
    std::vector<Locator_t> m_PendingLocators;
	uint32_t m_msgSize;

    TCPv6Connector(asio::io_service& io_service, const Locator_t& locator, uint32_t msgSize);
    ~TCPv6Connector();

    //! Method to start the connecting process with the endpoint set in the locator.
	void Connect(TCPv6Transport* parent, SenderResource *senderResource);

    //! Method to start the reconnection process.
	void RetryConnect(asio::io_service& io_service, TCPv6Transport* parent, SenderResource *senderResource);
};


/**
 * This is a default TCPv6 implementation.
 *    - Opening an output channel by passing a remote locator will try to open a TCP conection with the endpoint.
 *       If there is created a connection with the same endpoint, the transport will use the same one.
 *
 *    - It is possible to provide a white list at construction, which limits the interfaces the transport
 *       will ever be able to interact with. If left empty, all interfaces are allowed.
 *
 *    - Opening an input channel by passing a locator will open a socket listening on the given physical port on every
 *       whitelisted interface, it will wait for incomming connections until the receiver closes the channel.
 *       Several endpoints can connect to other to the same physical port, because the OS creates a connection socket
 *       after each establishment.
 * @ingroup TRANSPORT_MODULE
 */
class TCPv6Transport : public TCPTransportInterface
{
public:
    friend class RTCPMessageManager;
    friend class CleanTCPSocketsEvent;

    RTPS_DllAPI TCPv6Transport(const TCPv6TransportDescriptor&);

    virtual ~TCPv6Transport() override;

    bool init() override;

    //! Checks whether there are open and bound sockets for the given port.
    virtual bool IsInputChannelOpen(const Locator_t&) const override;

    //! Checks whether there are open and bound sockets for the given port.
    virtual bool IsOutputChannelOpen(const Locator_t&) const override;

    //! Checks if there is an opened socket or an acceptor waiting for connections.
    bool IsInputSocketOpen(const Locator_t&) const;

    //! Checks if the channel is bound to the given sender resource.
    bool IsOutputChannelBound(const Locator_t&) const;

    //! Checks if the channel is connected or the locator is bound to an input channel.
    bool IsOutputChannelConnected(const Locator_t&) const;

    //! Stores the binding between the given locator and the given TCP socket.
    virtual void BindSocket(const Locator_t&, TCPChannelResource*) override;

    //! Binds the output channel given by the locator to the sender resource.
    void BindOutputChannel(const Locator_t&, SenderResource *senderResource = nullptr);

    //! Unbind the given socket from every registered locator.
    void UnbindSocket(TCPChannelResource*);

    //! Checks for TCPv6 kind.
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

    /** Opens an input channel to receive incomming connections.
    *   If there is an existing channel it registers the receiver resource.
    */
    virtual bool OpenInputChannel(const Locator_t&, ReceiverResource*, uint32_t) override;

    //! Opens a socket on the given address and port (as long as they are white listed).
    virtual bool OpenOutputChannel(const Locator_t&, SenderResource*, uint32_t size = 0) override;

    //! Opens an additional output socket on the given address and port.
    virtual bool OpenExtraOutputChannel(Locator_t&, SenderResource*, uint32_t size = 0) override;

    //! Removes the listening socket for the specified port.
    virtual bool CloseInputChannel(const Locator_t&) override;

    //! Removes all outbound sockets on the given port.
    virtual bool CloseOutputChannel(const Locator_t&) override;

    /**
    * Blocking Send through the specified channel.
    * @param sendBuffer Slice into the raw data to send.
    * @param sendBufferSize Size of the raw data. It will be used as a bounds check for the previous argument.
    * It must not exceed the sendBufferSize fed to this class during construction.
    * @param localLocator Locator mapping to the channel we're sending from.
    * @param remoteLocator Locator describing the remote destination we're sending to.
    */
    virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator,
        const Locator_t& remoteLocator) override;

    /**
    * Blocking Send through the specified channel.
    * @param sendBuffer Slice into the raw data to send.
    * @param sendBufferSize Size of the raw data. It will be used as a bounds check for the previous argument.
    * It must not exceed the sendBufferSize fed to this class during construction.
    * @param localLocator Locator mapping to the channel we're sending from.
    * @param remoteLocator Locator describing the remote destination we're sending to.
    * @param pChannelResource Pointer to the socket to send the message.
    */
    virtual bool Send(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& localLocator,
        const Locator_t& remoteLocator, ChannelResource* pChannelResource) override;

    /**
    * Blocking Receive from the specified channel.
    * @param pChannelResource pointer to the socket where the method is going to read the messages.
    * @param receiveBuffer vector with enough capacity (not size) to accomodate a full receive buffer. That
    * capacity must not be less than the receiveBufferSize supplied to this class during construction.
    * @param receiveBufferCapacity maximum size of the buffer.
    * @param[out] receiveBufferSize Size of the packet received.
    * @param[out] logicalPort associated to the packet.
    */
    bool Receive(TCPChannelResource* pChannelResource, octet* receiveBuffer, uint32_t receiveBufferCapacity,
        uint32_t& receiveBufferSize, uint16_t& logicalPort);

    virtual LocatorList_t NormalizeLocator(const Locator_t& locator) override;

    virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;

    virtual bool is_local_locator(const Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override { return &mConfiguration_; }

    virtual void AddDefaultOutputLocator(LocatorList_t&) override;

    virtual uint16_t GetLogicalPortIncrement() const override;

    virtual uint16_t GetLogicalPortRange() const override;

    virtual uint16_t GetMaxLogicalPort() const override;

    //! Callback called each time that an incomming connection is accepted.
#ifdef ASIO_HAS_MOVE
    void SocketAccepted(TCPv6Acceptor* acceptor, const asio::error_code& error, asio::ip::tcp::socket s);
#else
    void SocketAccepted(TCPv6Acceptor* acceptor, const asio::error_code& error);
#endif

    //! Callback called each time that an outgoing connection is established.
    void SocketConnected(Locator_t& locator, SenderResource *senderResource, const asio::error_code& error);

protected:

    enum eSocketErrorCodes
    {
        eNoError,
        eBrokenPipe,
        eAsioError,
        eSystemError,
        eException
    };

    TCPv6TransportDescriptor mConfiguration_;
    RTCPMessageManager* mRTCPMessageManager;
    asio::io_service mService;
    std::shared_ptr<std::thread> ioServiceThread;
    std::vector<IPFinder::info_IP> mCurrentInterfaces;
    std::vector<asio::ip::address_v6> mInterfaceWhiteList;

    std::vector<TCPChannelResource*> mDeletedSocketsPool;
    std::recursive_mutex mDeletedSocketsPoolMutex;
    CleanTCPSocketsEvent* mCleanSocketsPoolTimer;

    mutable std::recursive_mutex mSocketsMapMutex;
    std::map<uint16_t, TCPv6Acceptor*> mSocketAcceptors; // The Key is the "Physical Port"
    std::map<uint16_t, std::vector<TCPChannelResource*>> mInputSockets; // The Key is the "Physical Port"
    std::map<Locator_t, ReceiverResource*> mReceiverResources;

    std::map<Locator_t, std::vector<SenderResource*>> mPendingOutputPorts;
    std::map<Locator_t, TCPv6Connector*> mSocketConnectors;
    std::vector<TCPChannelResource*> mOutputSockets;
    std::map<Locator_t, std::vector<TCPChannelResource*>> mBoundOutputSockets;
    mutable std::map<TCPChannelResource*, std::vector<SenderResource*>> mSocketToSenders;

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    TCPv6Transport();

    //! Cleans the sockets pending to delete.
    virtual void CleanDeletedSockets() override;

    //! Method to send the socket to the sender resource and avoid unnecessary searchs to send messages.
    void AssociateSenderToSocket(TCPChannelResource*, SenderResource*) const;

    //! Checks if the socket of the given locator has been opened as an input socket.
    bool IsTCPInputSocket(const Locator_t& locator) const;

    //! Checks if the given ip has been included in the white list to use it.
    bool IsInterfaceAllowed(const asio::ip::address_v6& ip);

    //! Intermediate method to open an output socket.
    bool OpenOutputSockets(const Locator_t& locator, SenderResource *senderResource, uint32_t maxMsgSize);

    //! Creates a TCP acceptor to wait for incomming connections by the given locator.
    bool CreateAcceptorSocket(const Locator_t& locator, uint32_t maxMsgSize);

    //! Method to create a TCP connector to establish a socket with the given locator.
	void CreateConnectorSocket(const Locator_t& locator, SenderResource *senderResource,
		std::vector<Locator_t>& pendingLocators, uint32_t msgSize);

    //! Adds the logical port of the given locator to send an Open Logical Port request.
    bool EnqueueLogicalOutputPort(const Locator_t& locator);

    //! Closes the given pChannelResource and unbind it from every resource.
    void CloseTCPSocket(TCPChannelResource* pChannelResource);

    //! Closes and removes the given socket from the input socket lists.
    void CloseInputSocket(TCPChannelResource* pChannelResource);

    //! Closes the physical socket and mark it to be deleted.
    void ReleaseTCPSocket(TCPChannelResource* pChannelResource);

    /** Associates the given pChannelResource with the registered ReceiverResources with the given locator.
    * This relationship is used to root the incomming messages to the related Receivers.
    */
    void RegisterReceiverResources(TCPChannelResource* pChannelResource, const Locator_t& locator);

    //! Functions to be called from new threads, which takes cares of performing a blocking receive
    void performListenOperation(TCPChannelResource* pChannelResource);
    void performRTPCManagementThread(TCPChannelResource* pChannelResource);

    bool ReadBody(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t* bytes_received,
        TCPChannelResource* pChannelResource, std::size_t body_size);

    //! Sends the given buffer by the given socket.
    bool SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator,
        TCPChannelResource* socket);

    size_t Send(TCPChannelResource* pChannelResource, const octet* data, size_t size, eSocketErrorCodes &error) const;
    virtual size_t Send(TCPChannelResource* pChannelResource, const octet* data, size_t size) const override;

    //! Methods to manage the TCP headers and their CRC values.
    bool CheckCRC(const TCPHeader &header, const octet *data, uint32_t size);
    void CalculateCRC(TCPHeader &header, const octet *data, uint32_t size);
    void FillTCPHeader(TCPHeader& header, const octet* sendBuffer, uint32_t sendBufferSize, uint16_t logicalPort);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
