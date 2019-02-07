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

#include <fastrtps/transport/TCPChannelResourceSecure.h>
#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/eClock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

using namespace tcp_secure;
using namespace asio;

TCPChannelResourceSecure::TCPChannelResourceSecure(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        asio::io_service& service,
        asio::ssl::context& ssl_context,
        const Locator_t& locator,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, rtcpManager, locator, maxMsgSize)
    , service_(service)
    , ssl_context_(ssl_context)
{
    secure_socket_ = createTCPSocket(service, ssl_context_);
    set_tls_verify_mode(parent->configuration());
}

TCPChannelResourceSecure::TCPChannelResourceSecure(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        asio::io_service& service,
        asio::ssl::context& ssl_context,
        tcp_secure::eProsimaTCPSocket socket,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, rtcpManager, maxMsgSize)
    , service_(service)
    , ssl_context_(ssl_context)
    , secure_socket_(socket)
{
    //set_tls_verify_mode(parent->configuration());
}

TCPChannelResourceSecure::~TCPChannelResourceSecure()
{
    // Take both mutexes to avoid the situation where
    // A checked alive and was true
    // A took mutex
    // B disables the channel resource
    // B destroy us
    // A tries to perform an operation causing calling a virtual method (our parent still lives).
    std::unique_lock<std::recursive_mutex> read_lock(read_mutex());
    std::unique_lock<std::recursive_mutex> write_lock(write_mutex());
}

void TCPChannelResourceSecure::connect()
{
    using asio::ip::tcp;
    std::unique_lock<std::mutex> scoped(status_mutex_);
    if (connection_status_ == eConnectionStatus::eDisconnected)
    {
        connection_status_ = eConnectionStatus::eConnecting;
        try
        {
            tcp::resolver resolver(service_);
            auto endpoints = resolver.resolve(IPLocator::toIPv4string(locator_),
                std::to_string(IPLocator::getPhysicalPort(locator_)));

            asio::async_connect(secure_socket_->lowest_layer(), endpoints,
                [this](const std::error_code& error,
                    const tcp::endpoint& /*endpoint*/)
            {
                if (!error)
                {
                    secure_socket_->async_handshake(ssl::stream_base::client,
                        [this](const std::error_code& error)
                    {
                        if (!error)
                        {
                            logInfo(RTCP_TLS, "Handshake sucessfull");
                            parent_->SocketConnected(locator_, error);
                        }
                        else
                        {
                            logError(RTCP_TLS, "Handshake failed: " << error.message());
                        }
                    });
                }
                else
                {
                    //logError(RTCP_TLS, "Connect failed: " << error.message());
                    parent_->SocketConnected(locator_, error); // Manages errors and retries
                }
            });
        }
        catch(const std::system_error &error)
        {
            logError(RTCP, "Openning socket " << error.what());
        }
    }
}

void TCPChannelResourceSecure::disconnect()
{
    if (change_status(eConnectionStatus::eDisconnected))
    {
        try
        {
            asio::error_code ec;
            secure_socket_->shutdown();
            secure_socket_->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            secure_socket_->lowest_layer().cancel();

          // This method was added on the version 1.12.0
#if ASIO_VERSION >= 101200 && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0603)
            secure_socket_->lowest_layer().release();
#endif
        }
        catch (std::exception&)
        {
            // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
        }
        secure_socket_->lowest_layer().close();
    }
}

uint32_t TCPChannelResourceSecure::read(
        octet* buffer,
        uint32_t buffer_capacity,
        std::size_t size)
{
    size_t rec = 0;

    while (rec < size && alive())
    {
        rec += secure_socket_->read_some(asio::buffer(buffer, buffer_capacity));
    }
    return rec;
}

uint32_t TCPChannelResourceSecure::read(
        octet* buffer,
        uint32_t buffer_capacity,
        std::size_t size,
        asio::error_code& ec)
{
    size_t rec = 0;

    while (rec < size && !ec && alive())
    {
        rec += secure_socket_->read_some(asio::buffer(buffer, buffer_capacity), ec);
    }
    return rec;
}

uint32_t TCPChannelResourceSecure::send(
        const octet* data,
        size_t size,
        asio::error_code& ec)
{
    size_t sent = 0;

    while (sent < size && !ec && alive())
    {
        sent += secure_socket_->write_some(asio::buffer(data, size), ec);
    }

    return sent;
}

asio::ip::tcp::endpoint TCPChannelResourceSecure::remote_endpoint() const
{
    return secure_socket_->lowest_layer().remote_endpoint();
}

asio::ip::tcp::endpoint TCPChannelResourceSecure::local_endpoint() const
{
    return secure_socket_->lowest_layer().local_endpoint();
}

void TCPChannelResourceSecure::set_options(const TCPTransportDescriptor* options)
{
    secure_socket_->lowest_layer().set_option(socket_base::receive_buffer_size(options->receiveBufferSize));
    secure_socket_->lowest_layer().set_option(socket_base::send_buffer_size(options->sendBufferSize));
    secure_socket_->lowest_layer().set_option(ip::tcp::no_delay(options->enable_tcp_nodelay));
}

void TCPChannelResourceSecure::set_tls_verify_mode(const TCPTransportDescriptor* options)
{
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;

    if (options->apply_security)
    {
        if (options->tls_config.verify_mode != TLSVerifyMode::UNUSED)
        {
            switch (options->tls_config.verify_mode)
            {
                case TLSVerifyMode::VERIFY_NONE:
                    secure_socket_->set_verify_mode(ssl::verify_none);
                    break;
                case TLSVerifyMode::VERIFY_PEER:
                    secure_socket_->set_verify_mode(ssl::verify_peer);
                    break;
                case TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT:
                    secure_socket_->set_verify_mode(ssl::verify_fail_if_no_peer_cert);
                    break;
                case TLSVerifyMode::VERIFY_CLIENT_ONCE:
                    secure_socket_->set_verify_mode(ssl::verify_client_once);
                    break;
                default:
                    break;
            }
        }
    }
}

void TCPChannelResourceSecure::cancel()
{
    secure_socket_->lowest_layer().cancel();
}

void TCPChannelResourceSecure::close()
{
    secure_socket_->lowest_layer().close();
}

void TCPChannelResourceSecure::shutdown(asio::socket_base::shutdown_type)
{
    secure_socket_->shutdown();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
