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
#include <fastrtps/transport/TCPChannelResourceBasic.h>
#include <fastrtps/transport/TCPAcceptorBasic.h>
#if TLS_FOUND
#include <fastrtps/transport/TCPAcceptorSecure.h>
#include <asio/ssl.hpp>
#endif


#include <asio.hpp>
#if TLS_FOUND
#include <asio/ssl.hpp>
#endif
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <fastrtps/utils/eClock.h> // Includes <windows.h> and may produce problems when included before asio.

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTCPMessageManager;
class CleanTCPSocketsEvent;
class TCPChannelResource;

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
    class ReceiverInUseCV
    {
        public:

            bool in_use = false;

            std::condition_variable cv;
    };

protected:
    std::vector<IPFinder::info_IP> current_interfaces_;
    int32_t transport_kind_;
    asio::io_service io_service_;
#if TLS_FOUND
    asio::ssl::context ssl_context_;
#endif
    std::shared_ptr<std::thread> io_service_thread_;
    RTCPMessageManager* rtcp_message_manager_;
    mutable std::mutex sockets_map_mutex_;
    std::atomic<bool> send_retry_active_;

    std::map<uint16_t, std::vector<TCPAcceptor*>> socket_acceptors_; // The Key is the "Physical Port"
    std::vector<TCPAcceptor*> deleted_acceptors_;
    std::map<Locator_t, TCPChannelResource*> channel_resources_; // The key is the "Physical locator"
    std::vector<TCPChannelResource*> unbound_channel_resources_; // Needed to avoid memory leaks if client doesn't bound
    // The key is the logical port
    std::map<uint16_t, std::pair<TransportReceiverInterface*, ReceiverInUseCV*>> receiver_resources_;

    std::vector<TCPChannelResource*> deleted_sockets_pool_;
    std::recursive_mutex deleted_sockets_pool_mutex_;
    CleanTCPSocketsEvent* clean_sockets_pool_timer_;

    std::vector<std::pair<TCPChannelResource*, uint64_t>> sockets_timestamp_;
    bool stop_socket_canceller_;
    std::mutex canceller_mutex_;
    eClock my_clock_;
    std::thread socket_canceller_thread_;

    TCPTransportInterface();

    virtual bool compare_locator_ip(
        const Locator_t& lh,
        const Locator_t& rh) const = 0;

    virtual bool compare_locator_ip_and_port(
        const Locator_t& lh,
        const Locator_t& rh) const = 0;

    virtual void fill_local_ip(Locator_t& loc) const = 0;

    //! Methods to manage the TCP headers and their CRC values.
    bool check_crc(
        const TCPHeader &header,
        const octet *data,
        uint32_t size) const;

    void calculate_crc(
        TCPHeader &header,
        const octet *data,
        uint32_t size) const;

    void fill_rtcp_header(
        TCPHeader& header,
        const octet* send_buffer,
        uint32_t send_buffer_size,
        uint16_t logical_port) const;

    //! Cleans the sockets pending to delete.
    void clean_deleted_sockets();

    //! Closes the given p_channel_resource and unbind it from every resource.
    void close_tcp_socket(TCPChannelResource* p_channel_resource);

    //! Creates a TCP acceptor to wait for incomming connections by the given locator.
    bool create_acceptor_socket(const Locator_t& locator);

    //! Method to create a TCP connector to establish a socket with the given locator.
    //void CreateConnectorSocket(const Locator_t& locator, SenderResource *senderResource,
    //    std::vector<Locator_t>& pendingLocators, uint32_t msgSize);

    //! Adds the logical port of the given locator to send an Open Logical Port request.
    bool enqueue_logical_output_port(const Locator_t& locator);

    virtual void get_ips(
        std::vector<IPFinder::info_IP>& loc_names,
        bool return_loopback = false) const = 0;

    //! Checks if the socket of the given locator has been opened as an input socket.
    bool is_tcp_input_socket(const Locator_t& locator) const;

    bool is_input_port_open(uint16_t port) const;

    //! Intermediate method to open an output socket.
    //bool OpenOutputSockets(const Locator_t& locator, SenderResource *senderResource);

    //! Functions to be called from new threads, which takes cares of performing a blocking receive
    void perform_listen_operation(TCPChannelResource* p_channel_resource);
    void perform_rtcp_management_thread(TCPChannelResource* p_channel_resource);

    bool read_body(
        octet* receive_buffer,
        uint32_t receive_buffer_capacity,
        uint32_t* bytes_received,
        TCPChannelResource* p_channel_resource,
        std::size_t body_size);
/*
    size_t send(
        TCPChannelResource* p_channel_resource,
        const octet* data,
        size_t size,
        eSocketErrorCodes &error) const;

    size_t send(
        TCPChannelResource* p_channel_resource,
        const octet* data,
        size_t size) const;

    //! Sends the given buffer by the given socket.
    bool send_through_socket(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        const Locator_t& remote_locator,
        TCPChannelResource* socket);
*/
    virtual void set_receive_buffer_size(uint32_t size) = 0;
    virtual void set_send_buffer_size(uint32_t size) = 0;

    void clean(); // Must be called on childs destructors!

    virtual void endpoint_to_locator(
        const asio::ip::tcp::endpoint& endpoint,
        Locator_t& locator) const = 0;

    /**
     * Shutdown method to close the connections of the transports.
    */
    virtual void shutdown() override;

    /**
     * Applies TLS configuration to ssl_context
     */
    void apply_tls_config();

    /**
     * Aux method to retrieve cert password as a callback
     */
    std::string get_password() const;

    void socket_canceller();

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
    virtual bool DoInputLocatorsMatch(
        const Locator_t&,
        const Locator_t&) const override;

    virtual bool DoOutputLocatorsMatch(
        const Locator_t&,
        const Locator_t&) const override;

    virtual asio::ip::tcp::endpoint generate_endpoint(uint16_t port) const = 0;

    virtual asio::ip::tcp::endpoint generate_endpoint(
        const Locator_t& loc,
        uint16_t port) const = 0;

    virtual asio::ip::tcp::endpoint generate_local_endpoint(
        Locator_t& loc,
        uint16_t port) const = 0;

    virtual asio::ip::tcp generate_protocol() const = 0;

    virtual asio::ip::tcp get_protocol_type() const = 0;

    virtual uint16_t GetLogicalPortIncrement() const  = 0;

    virtual uint16_t GetLogicalPortRange() const = 0;

    virtual uint16_t GetMaxLogicalPort() const = 0;

    bool init() override;

    //! Checks whether there are open and bound sockets for the given port.
    virtual bool IsInputChannelOpen(const Locator_t&) const override;

    //! Checks if the interfaces white list is empty.
    virtual bool is_interface_whitelist_empty() const = 0;

    /**
    * Checks if the given locator is allowed by the white list.
    * @param loc locator to check.
    * @return True if the locator passes the white list.
    */
    virtual bool is_interface_allowed(const Locator_t& loc) const = 0;

    virtual bool is_interface_allowed(const std::string& interface) const = 0;

    //! Checks for TCP kinds.
    virtual bool IsLocatorSupported(const Locator_t&) const override;

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
    virtual bool OpenInputChannel(
        const Locator_t&,
        TransportReceiverInterface*, uint32_t) override;

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
    * @param p_channel_resource pointer to the socket where the method is going to read the messages.
    * @param receive_buffer vector with enough capacity (not size) to accomodate a full receive buffer. That
    * capacity must not be less than the receive_buffer_size supplied to this class during construction.
    * @param receive_buffer_capacity maximum size of the buffer.
    * @param[out] receive_buffer_size Size of the packet received.
    * @param[out] remote_locator associated remote locator.
    */
    bool Receive(
        TCPChannelResource* p_channel_resource,
        octet* receive_buffer,
        uint32_t receive_buffer_capacity,
        uint32_t& receive_buffer_size,
        Locator_t& remote_locator);

    /**
    * Blocking Send through the specified channel.
    * @param send_buffer Slice into the raw data to send.
    * @param send_buffer_size Size of the raw data. It will be used as a bounds check for the previous argument.
    * It must not exceed the send_buffer_size fed to this class during construction.
    * @param localLocator Locator mapping to the channel we're sending from.
    * @param remote_locator Locator describing the remote destination we're sending to.
    */
    virtual bool send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        const Locator_t& localLocator,
        const Locator_t& remote_locator) override;

    /**
    * Blocking Send through the specified channel.
    * @param send_buffer Slice into the raw data to send.
    * @param send_buffer_size Size of the raw data. It will be used as a bounds check for the previous argument.
    * It must not exceed the send_buffer_size fed to this class during construction.
    * @param localLocator Locator mapping to the channel we're sending from.
    * @param remote_locator Locator describing the remote destination we're sending to.
    * @param p_channel_resource Pointer to the socket to send the message.
    */
    virtual bool send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        const Locator_t& localLocator,
        const Locator_t& remote_locator,
        ChannelResource* p_channel_resource) override;

    /**
     * Performs the locator selection algorithm for this transport.
     *
     * It basically constist of the following steps
     *   - selector.transport_starts is called
     *   - transport handles the selection state of each locator
     *   - if a locator from an entry is selected, selector.select is called for that entry
     *
     * In the case of TCP, multicast locators are never selected. All TCPv6 unicast locators 
     * are selected. For TCPv4, only locators on the same WAN as the transport or with the
     * WAN address of a connected channel are selected.
     *
     * @param [in, out] selector Locator selector.
     */
    virtual void select_locators(LocatorSelector& selector) const override;

    virtual LocatorList_t ShrinkLocatorLists(const std::vector<LocatorList_t>& locatorLists) override;

    //! Callback called each time that an incomming connection is accepted.
    void SocketAccepted(
        TCPAcceptorBasic* acceptor,
        Locator_t acceptor_locator,
        const asio::error_code& error);

#if TLS_FOUND
    //! Callback called each time that an incomming connection is accepted (secure).
    void SecureSocketAccepted(
        TCPAcceptorSecure* acceptor,
        Locator_t acceptor_locator,
        const asio::error_code& error);
#endif

    //! Callback called each time that an outgoing connection is established.
    void SocketConnected(
        Locator_t locator,
        const asio::error_code& error);

    //! Unbind the given socket from every registered locator.
    void UnbindSocket(TCPChannelResource*);

    /**
    * Method to get a list of interfaces to bind the socket associated to the given locator.
    * @param locator Input locator.
    * @return Vector of interfaces in string format.
    */
    virtual std::vector<std::string> get_binding_interfaces_list() = 0;

    virtual bool getDefaultMetatrafficMulticastLocators(
        LocatorList_t &locators,
        uint32_t metatraffic_multicast_port) const override;

    virtual bool getDefaultMetatrafficUnicastLocators(
        LocatorList_t &locators,
        uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(
        LocatorList_t &locators,
        uint32_t unicast_port) const override;

    virtual bool fillMetatrafficMulticastLocator(
        Locator_t &locator,
        uint32_t metatraffic_multicast_port) const override;

    virtual bool fillMetatrafficUnicastLocator(
        Locator_t &locator,
        uint32_t metatraffic_unicast_port) const override;

    virtual bool configureInitialPeerLocator(
        Locator_t &locator,
        const PortParameters &port_params,
        uint32_t domainId,
        LocatorList_t& list) const override;

    virtual bool fillUnicastLocator(
        Locator_t &locator,
        uint32_t well_known_port) const override;

    void DeleteSocket(TCPChannelResource *channelResource);

    virtual const TCPTransportDescriptor* configuration() const = 0;

    virtual TCPTransportDescriptor* configuration() = 0;

    void add_socket_to_cancel(
        TCPChannelResource* socket,
        uint64_t milliseconds);

    void remove_socket_to_cancel(TCPChannelResource* socket);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // TCP_TRANSPORT_INTERFACE_H
