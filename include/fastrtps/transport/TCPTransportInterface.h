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

#ifndef TCP_TRANSPORT_INTERFACE_H
#define TCP_TRANSPORT_INTERFACE_H

#include <fastrtps/transport/TransportInterface.h>
#include <fastrtps/transport/TCPTransportDescriptor.h>
#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/transport/tcp/RTCPHeader.h>
#include <fastrtps/transport/TCPChannelResource.h>

#include <asio.hpp>
#include <thread>
#include <vector>
#include <map>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTCPMessageManager;
class CleanTCPSocketsEvent;
class TCPChannelResource;
class TCPTransportInterface;

class TCPAcceptor
{
public:
    asio::ip::tcp::acceptor mAcceptor;
    Locator_t mLocator;
    eProsimaTCPSocket mSocket;
    asio::ip::tcp::endpoint mEndPoint;
    std::vector<Locator_t> mPendingOutLocators;

    /**
    * Constructor
    * @param io_service Reference to the ASIO service.
    * @param parent Pointer to the transport that is going to manage the acceptor.
    * @param locator Locator with the information about where to accept connections.
    */
    TCPAcceptor(asio::io_service& io_service, TCPTransportInterface* parent, const Locator_t& locator);

    /**
    * Constructor
    * @param io_service Reference to the ASIO service.
    * @param sInterface Network interface to bind the socket
    * @param locator Locator with the information about where to accept connections.
    */
    TCPAcceptor(asio::io_service& io_service, const std::string& sInterface, const Locator_t& locator);

    /**
    * Destructor
    */
    ~TCPAcceptor()
    {
        try { asio::error_code ec; mSocket.cancel(ec); }
        catch (...) {}
        mSocket.close();
    }

    //! Method to start the accepting process.
    void Accept(TCPTransportInterface* parent, asio::io_service&);
};

/**
 * This is a default TCP Interface implementation.
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
class TCPTransportInterface : public TransportInterface
{
public:
    friend class RTCPMessageManager;
    friend class test_RTCPMessageManager;
    friend class CleanTCPSocketsEvent;

    virtual ~TCPTransportInterface();

    //! Stores the binding between the given locator and the given TCP socket.
    TCPChannelResource* BindSocket(const Locator_t&, TCPChannelResource*);

    //! Removes the listening socket for the specified port.
    virtual bool CloseInputChannel(const Locator_t&) override;

    //! Removes all outbound sockets on the given port.
    virtual bool CloseOutputChannel(const Locator_t&) override;

    //! Reports whether Locators correspond to the same port.
    virtual bool DoInputLocatorsMatch(const Locator_t&, const Locator_t&) const override;
    virtual bool DoOutputLocatorsMatch(const Locator_t&, const Locator_t&) const override;

    virtual asio::ip::tcp::endpoint GenerateEndpoint(uint16_t port) const = 0;
    virtual asio::ip::tcp::endpoint GenerateEndpoint(const Locator_t& loc, uint16_t port) const = 0;
    virtual asio::ip::tcp::endpoint GenerateLocalEndpoint(Locator_t& loc, uint16_t port) const = 0;
    virtual asio::ip::tcp GenerateProtocol() const = 0;


    virtual asio::ip::tcp GetProtocolType() const = 0;

    virtual uint16_t GetLogicalPortIncrement() const  = 0;

    virtual uint16_t GetLogicalPortRange() const = 0;

    virtual uint16_t GetMaxLogicalPort() const = 0;

    bool init() override;

    //! Checks whether there are open and bound sockets for the given port.
    virtual bool IsInputChannelOpen(const Locator_t&) const override;

    //! Checks if the interfaces white list is empty.
    virtual bool IsInterfaceWhiteListEmpty() const = 0;

    /**
    * Checks if the given locator is allowed by the white list.
    * @param loc locator to check.
    * @return True if the locator passes the white list.
    */
    virtual bool IsInterfaceAllowed(const Locator_t& loc) const = 0;

    //! Checks for TCP kinds.
    virtual bool IsLocatorSupported(const Locator_t&) const override;
    virtual bool IsInterfaceAllowed(const std::string& interface) const = 0;

    //! Checks if the channel is bound to the given sender resource.
    bool IsOutputChannelBound(const Locator_t&) const;

    //! Checks if the channel is connected or the locator is bound to an input channel.
    bool IsOutputChannelConnected(const Locator_t&) const;

    //! Checks whether there are open and bound sockets for the given port.
    virtual bool IsOutputChannelOpen(const Locator_t&) const override;

    //! Opens an additional output socket on the given address and port.
    virtual bool OpenExtraOutputChannel(const Locator_t&) override;

    /** Opens an input channel to receive incomming connections.
    *   If there is an existing channel it registers the receiver resource.
    */
    virtual bool OpenInputChannel(const Locator_t&, TransportReceiverInterface*, uint32_t) override;

    //! Opens a socket on the given address and port (as long as they are white listed).
    virtual bool OpenOutputChannel(const Locator_t&) override;

    /**
    * Converts a given remote locator (that is, a locator referring to a remote
    * destination) to the main local locator whose channel can write to that
    * destination. In this case it will return a 0.0.0.0 address on that port.
    */
    virtual Locator_t RemoteToMainLocal(const Locator_t&) const override;

    /**
    * Blocking Receive from the specified channel.
    * @param pChannelResource pointer to the socket where the method is going to read the messages.
    * @param receiveBuffer vector with enough capacity (not size) to accomodate a full receive buffer. That
    * capacity must not be less than the receiveBufferSize supplied to this class during construction.
    * @param receiveBufferCapacity maximum size of the buffer.
    * @param[out] receiveBufferSize Size of the packet received.
    * @param[out] remoteLocator associated remote locator.
    */
    bool Receive(TCPChannelResource* pChannelResource, octet* receiveBuffer, uint32_t receiveBufferCapacity,
        uint32_t& receiveBufferSize, Locator_t& remoteLocator);

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

    virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;

    //! Callback called each time that an incomming connection is accepted.
    void SocketAccepted(TCPAcceptor* acceptor, const asio::error_code& error);

    //! Callback called each time that an outgoing connection is established.
    void SocketConnected(Locator_t locator, const asio::error_code& error);

    //! Unbind the given socket from every registered locator.
    void UnbindSocket(TCPChannelResource*);

    /**
    * Method to get a list of interfaces to bind the socket associated to the given locator.
    * @param locator Input locator.
    * @return Vector of interfaces in string format.
    */
    virtual std::vector<std::string> GetBindingInterfacesList() = 0;

    virtual bool getDefaultMetatrafficMulticastLocators(LocatorList_t &locators,
        uint32_t metatraffic_multicast_port) const override;

    virtual bool getDefaultMetatrafficUnicastLocators(LocatorList_t &locators,
        uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(LocatorList_t &locators, uint32_t unicast_port) const override;

    virtual bool fillMetatrafficMulticastLocator(Locator_t &locator,
        uint32_t metatraffic_multicast_port) const override;

    virtual bool fillMetatrafficUnicastLocator(Locator_t &locator, uint32_t metatraffic_unicast_port) const override;

    virtual bool configureInitialPeerLocator(Locator_t &locator, const PortParameters &port_params, uint32_t domainId,
        LocatorList_t& list) const override;

    virtual bool fillUnicastLocator(Locator_t &locator, uint32_t well_known_port) const override;

    void DeleteSocket(TCPChannelResource *channelResource);

private:

    class ReceiverInUseCV
    {
        public:

            bool in_use = false;

            std::condition_variable cv;
    };

protected:

    std::vector<IPFinder::info_IP> mCurrentInterfaces;
    int32_t mTransportKind;
    asio::io_service mService;
    std::shared_ptr<std::thread> ioServiceThread;
    RTCPMessageManager* mRTCPMessageManager;
    mutable std::mutex mSocketsMapMutex;
    std::atomic<bool> mSendRetryActive;

    std::map<uint16_t, std::vector<TCPAcceptor*>> mSocketAcceptors; // The Key is the "Physical Port"
    std::vector<TCPAcceptor*> mDeletedAcceptors;
    std::map<Locator_t, TCPChannelResource*> mChannelResources; // The key is the "Physical locator"
    std::vector<TCPChannelResource*> mUnboundChannelResources; // Needed to avoid memory leaks if client doesn't bound
    // The key is the logical port
    std::map<uint16_t, std::pair<TransportReceiverInterface*, ReceiverInUseCV*>> mReceiverResources;

    std::vector<TCPChannelResource*> mDeletedSocketsPool;
    std::recursive_mutex mDeletedSocketsPoolMutex;
    CleanTCPSocketsEvent* mCleanSocketsPoolTimer;

    std::map<Locator_t, std::vector<SenderResource*>> mPendingOutputPorts;

    TCPTransportInterface();

    virtual bool CompareLocatorIP(const Locator_t& lh, const Locator_t& rh) const = 0;
    virtual bool CompareLocatorIPAndPort(const Locator_t& lh, const Locator_t& rh) const = 0;

    virtual void FillLocalIp(Locator_t& loc) const = 0;

    //! Methods to manage the TCP headers and their CRC values.
    bool CheckCRC(const TCPHeader &header, const octet *data, uint32_t size) const;
    void CalculateCRC(TCPHeader &header, const octet *data, uint32_t size) const;
    void FillTCPHeader(TCPHeader& header, const octet* sendBuffer, uint32_t sendBufferSize, uint16_t logicalPort) const;

    //! Cleans the sockets pending to delete.
    void CleanDeletedSockets();

    //! Closes the given pChannelResource and unbind it from every resource.
    void CloseTCPSocket(TCPChannelResource* pChannelResource);

    //! Creates a TCP acceptor to wait for incomming connections by the given locator.
    bool CreateAcceptorSocket(const Locator_t& locator);

    //! Method to create a TCP connector to establish a socket with the given locator.
    void CreateConnectorSocket(const Locator_t& locator, SenderResource *senderResource,
        std::vector<Locator_t>& pendingLocators, uint32_t msgSize);

    //! Adds the logical port of the given locator to send an Open Logical Port request.
    bool EnqueueLogicalOutputPort(const Locator_t& locator);

    virtual const TCPTransportDescriptor* GetConfiguration() const = 0;
    virtual TCPTransportDescriptor* GetConfiguration() = 0;

    virtual void GetIPs(std::vector<IPFinder::info_IP>& locNames, bool return_loopback = false) const = 0;

    //! Checks if the socket of the given locator has been opened as an input socket.
    bool IsTCPInputSocket(const Locator_t& locator) const;

    bool IsInputPortOpen(uint16_t port) const;

    //! Intermediate method to open an output socket.
    bool OpenOutputSockets(const Locator_t& locator, SenderResource *senderResource);

    //! Functions to be called from new threads, which takes cares of performing a blocking receive
    void performListenOperation(TCPChannelResource* pChannelResource);
    void performRTPCManagementThread(TCPChannelResource* pChannelResource);

    bool ReadBody(octet* receiveBuffer, uint32_t receiveBufferCapacity, uint32_t* bytes_received,
        TCPChannelResource* pChannelResource, std::size_t body_size);

    size_t Send(TCPChannelResource* pChannelResource, const octet* data, size_t size, eSocketErrorCodes &error) const;
    size_t Send(TCPChannelResource* pChannelResource, const octet* data, size_t size) const;

    //! Sends the given buffer by the given socket.
    bool SendThroughSocket(const octet* sendBuffer, uint32_t sendBufferSize, const Locator_t& remoteLocator,
        TCPChannelResource* socket);

    virtual void SetReceiveBufferSize(uint32_t size) = 0;
    virtual void SetSendBufferSize(uint32_t size) = 0;

    void Clean(); // Must be called on childs destructors!

    virtual void EndpointToLocator(const asio::ip::tcp::endpoint& endpoint, Locator_t& locator) const = 0;

    /**
     * Shutdown method to close the connections of the transports.
    */
    virtual void Shutdown() override;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // TCP_TRANSPORT_INTERFACE_H
