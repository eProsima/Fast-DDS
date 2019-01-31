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

#ifndef _TCP_CHANNEL_RESOURCE_SECURE_
#define _TCP_CHANNEL_RESOURCE_SECURE_

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <fastrtps/transport/TCPChannelResource.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

namespace tcp_secure{
    // Typedefs
	typedef std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> eProsimaTCPSocket;
    typedef eProsimaTCPSocket eProsimaTCPSocketRef;

    inline eProsimaTCPSocket getSocketPtr(eProsimaTCPSocket socket)
    {
        return socket;
    }

    inline eProsimaTCPSocket moveSocket(eProsimaTCPSocket socket)
    {
        return socket;
    }

    inline eProsimaTCPSocket createTCPSocket(
        asio::io_service& io_service,
        asio::ssl::context& ssl_context)
    {
        return std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(io_service, ssl_context);
    }

    inline eProsimaTCPSocket createTCPSocket(
        asio::ip::tcp::socket&& socket,
        asio::ssl::context& ssl_context)
    {
        return std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(socket), ssl_context);
    }

	inline asio::ssl::stream<asio::ip::tcp::socket>& getTCPSocketRef(eProsimaTCPSocket socket)
    {
        return *socket;
    }
} // namespace tcp_secure

class TCPChannelResourceSecure : public TCPChannelResource
{
    asio::io_service& service_;
    asio::ssl::context& ssl_context_;
    tcp_secure::eProsimaTCPSocket secure_socket_;

public:
    // Constructor called when trying to connect to a remote server (secure version)
    TCPChannelResourceSecure(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        asio::io_service& service,
        asio::ssl::context& ssl_context,
        const Locator_t& locator,
        uint32_t maxMsgSize);

    // Constructor called when local server accepted connection (secure version)
    TCPChannelResourceSecure(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        asio::io_service& service,
        asio::ssl::context& ssl_context,
        asio::ip::tcp::socket&& socket,
        uint32_t maxMsgSize);

    virtual ~TCPChannelResourceSecure();

    void connect() override;

    void disconnect() override;

    uint32_t read(
        octet* buffer,
        uint32_t buffer_capacity,
        std::size_t size) override;

    uint32_t read(
        octet* buffer,
        uint32_t buffer_capacity,
        std::size_t size,
        asio::error_code& ec) override;

    uint32_t send(
        const octet* data,
        size_t size,
        asio::error_code& ec) override;

    asio::ip::tcp::endpoint remote_endpoint() const override;
    asio::ip::tcp::endpoint local_endpoint() const override;

    void set_options(const TCPTransportDescriptor* options) override;

    void cancel() override;
    void close() override;
    void shutdown(asio::socket_base::shutdown_type what) override;

    inline tcp_secure::eProsimaTCPSocket secure_socket()
    {
        return tcp_secure::getSocketPtr(secure_socket_);
    }

protected:
    void apply_tls_config();
    std::string get_password() const;

private:
    TCPChannelResourceSecure(const TCPChannelResource&) = delete;
    TCPChannelResourceSecure& operator=(const TCPChannelResource&) = delete;
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _TCP_CHANNEL_RESOURCE_SECURE_
