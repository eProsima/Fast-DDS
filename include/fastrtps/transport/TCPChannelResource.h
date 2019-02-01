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

#ifndef _TCP_CHANNEL_RESOURCE_BASE_
#define _TCP_CHANNEL_RESOURCE_BASE_

#include <fastrtps/transport/TCPTransportDescriptor.h>
#include <fastrtps/transport/TransportReceiverInterface.h>
#include <fastrtps/transport/ChannelResource.h>
#include <fastrtps/transport/tcp/RTCPMessageManager.h>
#include <fastrtps/rtps/common/Locator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

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
    RTCPMessageManager* rtcp_manager_;
    Locator_t locator_;
    bool input_socket_;
    bool waiting_for_keep_alive_;
    // Must be accessed after lock pending_logical_mutex_
    std::map<TCPTransactionId, uint16_t> negotiating_logical_ports_;
    std::map<TCPTransactionId, uint16_t> last_checked_logical_port_;
    std::thread* rtcp_thread_;
    std::vector<uint16_t> pending_logical_output_ports_; // Must be accessed after lock pending_logical_mutex_
    std::vector<uint16_t> logical_output_ports_;
    std::recursive_mutex read_mutex_;
    std::recursive_mutex write_mutex_;
    std::recursive_mutex pending_logical_mutex_;
    std::condition_variable negotiation_condition_;
    eConnectionStatus connection_status_;
    std::mutex status_mutex_;

public:
    void add_logical_port(uint16_t port);

    void set_logical_port_pending(uint16_t port);

    bool remove_logical_port(uint16_t port);

	virtual void disable() override;

    std::recursive_mutex& read_mutex()
    {
        return read_mutex_;
    }

    std::recursive_mutex& write_mutex()
    {
        return write_mutex_;
    }

    inline void rtcp_thread(std::thread* pThread)
    {
        rtcp_thread_ = pThread;
    }

    std::thread* release_rtcp_thread();

    inline bool input_socket() const
    {
        return input_socket_;
    }

	bool is_logical_port_opened(uint16_t port);

    bool is_logical_port_added(uint16_t port);

    bool connection_established()
    {
        std::unique_lock<std::mutex> scoped(status_mutex_);
        return connection_status_ == eConnectionStatus::eEstablished;
    }

    inline const Locator_t& locator() const
    {
        return locator_;
    }

    void input_port_closed(uint16_t port);

    ResponseCode process_bind_request(const Locator_t& locator);

    // Socket related methods
    virtual void connect() = 0;

    virtual void disconnect() = 0;

    virtual uint32_t read(
        octet* buffer,
        uint32_t buffer_capacity,
        std::size_t size) = 0;

    virtual uint32_t read(
        octet* buffer,
        uint32_t buffer_capacity,
        std::size_t size,
        asio::error_code& ec) = 0;

    virtual uint32_t send(
        const octet* buffer,
        size_t size,
        asio::error_code& ec) = 0;

    virtual asio::ip::tcp::endpoint remote_endpoint() const = 0;

    virtual asio::ip::tcp::endpoint local_endpoint() const = 0;

    virtual void set_options(const TCPTransportDescriptor* options) = 0;

    virtual void cancel() = 0;

    virtual void close() = 0;

    virtual void shutdown(asio::socket_base::shutdown_type what) = 0;

    bool wait_until_port_is_open_or_connection_is_closed(uint16_t port);

    virtual ~TCPChannelResource();

protected:
    // Constructor called when trying to connect to a remote server
    TCPChannelResource(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        const Locator_t& locator,
        uint32_t maxMsgSize);

    // Constructor called when local server accepted connection
    TCPChannelResource(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        uint32_t maxMsgSize);

    inline bool change_status(eConnectionStatus s)
    {
        std::unique_lock<std::mutex> scoped(status_mutex_);
        if (connection_status_ != s)
        {
        	connection_status_ = s;
	        if (connection_status_ == eEstablished)
	        {
	            send_pending_open_logical_ports();
	        }
            negotiation_condition_.notify_all();
	        return true;
	    }
	    return false;
    }

    void add_logical_port_response(const TCPTransactionId &id, bool success);

    void process_check_logical_ports_response(const TCPTransactionId &transactionId,
        const std::vector<uint16_t> &availablePorts);

    friend class TCPTransportInterface;
    friend class RTCPMessageManager;
    friend class test_RTCPMessageManager;

private:
    void prepare_send_check_logical_ports_req(uint16_t closedPort);

    void send_pending_open_logical_ports();

    void copy_pending_ports_from(TCPChannelResource* from);

    void set_all_ports_pending();

    TCPChannelResource(const TCPChannelResource&) = delete;

    TCPChannelResource& operator=(const TCPChannelResource&) = delete;
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _TCP_CHANNEL_RESOURCE_BASE_
