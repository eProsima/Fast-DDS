// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_TCP_CHANNEL_RESOURCE_BASE_
#define _FASTDDS_TCP_CHANNEL_RESOURCE_BASE_

#include <asio.hpp>
#include <fastdds/rtps/transport/TCPTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TransportReceiverInterface.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <rtps/transport/ChannelResource.h>
#include <rtps/transport/tcp/RTCPMessageManager.h>


namespace eprosima {
namespace fastdds {
namespace rtps {

class TCPConnector;
class TCPTransportInterface;

enum eSocketErrorCodes
{
    eNoError,
    eBrokenPipe,
    eAsioError,
    eSystemError,
    eException,
    eConnectionAborted = 125
};

class TCPChannelResource : public ChannelResource
{

protected:

    enum TCPConnectionType
    {
        TCP_ACCEPT_TYPE = 0,
        TCP_CONNECT_TYPE = 1
    };

    enum eConnectionStatus
    {
        eDisconnected = 0,
        eConnecting,                // Output -> Trying connection.
        eConnected,                 // Output -> Send bind message.
        eWaitingForBind,            // Input -> Waiting for the bind message.
        eWaitingForBindResponse,    // Output -> Waiting for the bind response message.
        eEstablished,
        eUnbinding
    };

    TCPTransportInterface* parent_;
    Locator locator_;
    bool waiting_for_keep_alive_;
    // Must be accessed after lock pending_logical_mutex_
    std::map<TCPTransactionId, uint16_t> negotiating_logical_ports_;
    std::map<TCPTransactionId, uint16_t> last_checked_logical_port_;
    std::vector<uint16_t> pending_logical_output_ports_; // Must be accessed after lock pending_logical_mutex_
    std::vector<uint16_t> logical_output_ports_;
    std::condition_variable_any logical_output_ports_updated_cv;
    std::mutex read_mutex_;
    std::recursive_mutex pending_logical_mutex_;
    std::atomic<eConnectionStatus> connection_status_;

public:

    void add_logical_port(
            uint16_t port,
            RTCPMessageManager* rtcp_manager);

    void set_logical_port_pending(
            uint16_t port);

    bool remove_logical_port(
            uint16_t port);

    virtual void disable() override;

    bool is_logical_port_opened(
            uint16_t port);

    bool is_logical_port_added(
            uint16_t port);

    /**
     * This method checks if a logical port is under negotiation. If it is, it waits for the negotiation to finish up to a timeout.
     * Independently if being under negotiation or not, it returns true if the port is opened, false otherwise.
     *
     * @param port The logical port to check.
     * @param timeout The maximum time to wait for the negotiation to finish. Zero value means no wait
     *
     * @return true if the port is opened, false otherwise.
     */
    bool wait_logical_port_under_negotiation(
            uint16_t port,
            const std::chrono::milliseconds& timeout);

    bool connection_established()
    {
        return connection_status_ == eConnectionStatus::eEstablished;
    }

    eConnectionStatus connection_status()
    {
        return connection_status_;
    }

    inline const Locator& locator() const
    {
        return locator_;
    }

    ResponseCode process_bind_request(
            const Locator& locator);

    // Socket related methods
    virtual void connect(
            const std::shared_ptr<TCPChannelResource>& myself) = 0;

    virtual void disconnect() = 0;

    virtual uint32_t read(
            octet* buffer,
            std::size_t size,
            asio::error_code& ec) = 0;

    /**
     * Sends the provided TCP header and data over the TCP channel.
     * Used solely during TCP connection negotiations.
     *
     * @param header Pointer to the octet array containing the TCP header.
     * @param header_size Size of the TCP header array.
     * @param data Pointer to the octet array containing the data.
     * @param data_size Size of the data array.
     * @param ec Reference to the asio::error_code object to store any error that occurs during the send operation.
     * @return The number of bytes actually sent.
     */
    size_t send(
            const fastdds::rtps::octet* header,
            size_t header_size,
            const fastdds::rtps::octet* data,
            uint32_t data_size,
            asio::error_code& ec)
    {
        NetworkBuffer buffers(data, data_size);
        std::vector<NetworkBuffer> buffer_list;
        buffer_list.push_back(buffers);
        return send(header, header_size, buffer_list, data_size, ec);
    }

    /**
     * Sends the provided TCP header and data over the TCP channel.
     * Used for TCP metatraffic and data transmission.
     *
     * @param header Pointer to the TCP header data.
     * @param header_size Size of the TCP header data.
     * @param buffers Vector of network buffers containing the data to be sent.
     * @param total_bytes Total number of bytes to be sent.
     * @param ec Reference to an asio::error_code object to store any error that occurs during the send operation.
     * @return The number of bytes actually sent.
     */
    virtual size_t send(
            const octet* header,
            size_t header_size,
            const std::vector<NetworkBuffer>& buffers,
            uint32_t total_bytes,
            asio::error_code& ec) = 0;

    /**
     * @brief Gets the remote endpoint of the socket connection.
     * @throws Exception on failure.
     * @return asio::ip::tcp::endpoint of the remote endpoint.
     */
    virtual asio::ip::tcp::endpoint remote_endpoint() const = 0;

    /**
     * @brief Gets the local endpoint of the socket connection.
     * @throws Exception on failure.
     * @return asio::ip::tcp::endpoint of the local endpoint.
     */
    virtual asio::ip::tcp::endpoint local_endpoint() const = 0;

    /**
     * @brief Gets the remote endpoint, setting error code if any.
     * @param ec Set to indicate what error occurred, if any.
     * @return asio::ip::tcp::endpoint of the remote endpoint or returns a default-constructed endpoint object if an error occurred.
     */
    virtual asio::ip::tcp::endpoint remote_endpoint(
            asio::error_code& ec) const = 0;

    /**
     * @brief Gets the local endpoint, setting error code if any.
     * @param ec Set to indicate what error occurred, if any.
     * @return asio::ip::tcp::endpoint of the remote endpoint or returns a default-constructed endpoint object if an error occurred.
     */
    virtual asio::ip::tcp::endpoint local_endpoint(
            asio::error_code& ec) const = 0;

    virtual void set_options(
            const TCPTransportDescriptor* options) = 0;

    virtual void cancel() = 0;

    virtual void close() = 0;

    virtual void shutdown(
            asio::socket_base::shutdown_type what) = 0;

    TCPConnectionType tcp_connection_type() const
    {
        return tcp_connection_type_;
    }

protected:

    // Constructor called when trying to connect to a remote server
    TCPChannelResource(
            TCPTransportInterface* parent,
            const Locator& locator,
            uint32_t maxMsgSize);

    // Constructor called when local server accepted connection
    TCPChannelResource(
            TCPTransportInterface* parent,
            uint32_t maxMsgSize);

    inline eConnectionStatus change_status(
            eConnectionStatus s,
            RTCPMessageManager* rtcp_manager = nullptr)
    {
        eConnectionStatus old = connection_status_.exchange(s);

        if (old != s)
        {
            if (s == eEstablished)
            {
                assert(rtcp_manager != nullptr);
                send_pending_open_logical_ports(rtcp_manager);
            }
        }

        return old;
    }

    void add_logical_port_response(
            const TCPTransactionId& id,
            bool success,
            RTCPMessageManager* rtcp_manager);

    void process_check_logical_ports_response(
            const TCPTransactionId& transactionId,
            const std::vector<uint16_t>& availablePorts,
            RTCPMessageManager* rtcp_manager);

    bool check_socket_send_buffer(
            const size_t& msg_size,
            const asio::ip::tcp::socket::native_handle_type& socket_native_handle);

    /**
     * @brief Set descriptor options on a socket.
     *
     * @param socket Socket on which to set the options.
     * @param options Descriptor with the options to set.
     */
    static void set_socket_options(
            asio::basic_socket<asio::ip::tcp>& socket,
            const TCPTransportDescriptor* options);

    TCPConnectionType tcp_connection_type_;

    friend class TCPTransportInterface;
    friend class RTCPMessageManager;

private:

    bool is_logical_port_opened_nts(
            uint16_t port);

    void prepare_send_check_logical_ports_req(
            uint16_t closedPort,
            RTCPMessageManager* rtcp_manager);

    void send_pending_open_logical_ports(
            RTCPMessageManager* rtcp_manager);

    void set_all_ports_pending();

    TCPChannelResource(
            const TCPChannelResource&) = delete;

    TCPChannelResource& operator =(
            const TCPChannelResource&) = delete;
};


} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TCP_CHANNEL_RESOURCE_BASE_
