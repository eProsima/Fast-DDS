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
    apply_tls_config();
    secure_socket_ = createTCPSocket(service, ssl_context_);
    set_tls_verify_mode(parent->configuration());
}

TCPChannelResourceSecure::TCPChannelResourceSecure(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        asio::io_service& service,
        asio::ssl::context& ssl_context,
        asio::ip::tcp::socket&& socket,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, rtcpManager, maxMsgSize)
    , service_(service)
    , ssl_context_(ssl_context)
    //, secure_socket_(moveSocket(socket))
{
    apply_tls_config();
    secure_socket_ = createTCPSocket(std::move(socket), ssl_context_);
    set_tls_verify_mode(parent->configuration());
}

TCPChannelResourceSecure::~TCPChannelResourceSecure()
{
}

void TCPChannelResourceSecure::apply_tls_config()
{
    const TCPTransportDescriptor* descriptor = parent_->configuration();
    if (descriptor->apply_security)
    {
        const TCPTransportDescriptor::TLSConfig* config = &descriptor->tls_config;
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;

        /*
        uint32_t options;
        */

        if (!config->password.empty())
        {
            ssl_context_.set_password_callback(std::bind(&TCPChannelResourceSecure::get_password, this));
        }

        if (!config->verify_file.empty())
        {
            ssl_context_.load_verify_file(config->verify_file);
        }

        if (!config->cert_chain_file.empty())
        {
            ssl_context_.use_certificate_chain_file(config->cert_chain_file);
        }

        if (!config->private_key_file.empty())
        {
            ssl_context_.use_private_key_file(config->private_key_file, ssl::context::pem);
        }

        if (!config->tmp_dh_file.empty())
        {
            ssl_context_.use_tmp_dh_file(config->tmp_dh_file);
        }

        if (config->options != TLSOptions::NONE)
        {
            uint32_t options = 0;

            if (config->get_option(TLSOptions::DEFAULT_WORKAROUNDS))
            {
                options |= ssl::context::default_workarounds;
            }

            if (config->get_option(TLSOptions::NO_COMPRESSION))
            {
                options |= ssl::context::no_compression;
            }

            if (config->get_option(TLSOptions::NO_SSLV2))
            {
                options |= ssl::context::no_sslv2;
            }

            if (config->get_option(TLSOptions::NO_SSLV3))
            {
                options |= ssl::context::no_sslv3;
            }

            if (config->get_option(TLSOptions::NO_TLSV1))
            {
                options |= ssl::context::no_tlsv1;
            }

            if (config->get_option(TLSOptions::NO_TLSV1_1))
            {
                options |= ssl::context::no_tlsv1_1;
            }

            if (config->get_option(TLSOptions::NO_TLSV1_2))
            {
                options |= ssl::context::no_tlsv1_2;
            }

            // if (config->get_option(TLSOptions::NO_TLSV1_3))
            // {
            //     options |= ssl::context::no_tlsv1_3; // Asio needs to be updated
            // }

            if (config->get_option(TLSOptions::SINGLE_DH_USE))
            {
                options |= ssl::context::single_dh_use;
            }

            ssl_context_.set_options(options);
        }
    }
}

void TCPChannelResourceSecure::connect()
{
    std::unique_lock<std::mutex> scoped(status_mutex_);
    if (connection_status_ == eConnectionStatus::eDisconnected)
    {
        connection_status_ = eConnectionStatus::eConnecting;
        ip::tcp::endpoint endpoint = parent_->generate_local_endpoint(locator_, IPLocator::getPhysicalPort(locator_));
        try
        {
            secure_socket_->lowest_layer().async_connect(endpoint,
                [this](const std::error_code& error)
                {
                    // handshake as client and call TCPTransportInterface::SocketConnected
                    if (!error)
                    {
                        secure_socket_->async_handshake(ssl::stream_base::client,
                            [this](const std::error_code& error)
                            {
                                logError(TLS_CLIENT, error.message());
                                parent_->SocketConnected(locator_, error);
                            });
                    }
                    else
                    {
                        logError(RTCP_TLS, error.message());
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
            //secure_socket_->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            //secure_socket_->lowest_layer().cancel();

          // This method was added on the version 1.12.0
#if ASIO_VERSION >= 101200 && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0603)
            //secure_socket_->lowest_layer().release();
#endif
        }
        catch (std::exception&)
        {
            // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
        }
        //secure_socket_->lowest_layer().close();
    }
}

uint32_t TCPChannelResourceSecure::read(
        octet* buffer,
        uint32_t buffer_capacity,
        std::size_t size)
{
    size_t rec = 0;

    while (rec < size)
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
    while (rec < size && !ec)
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

    while (sent < size && !ec)
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
    secure_socket_->set_verify_mode(ssl::verify_peer);
    secure_socket_->set_verify_callback([](
            bool preverified,
            ssl::verify_context& ctx) -> bool
    {
        return true;
    });
    return;

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
    //secure_socket_->lowest_layer().cancel();
}

void TCPChannelResourceSecure::close()
{
    //secure_socket_->lowest_layer().close();
}

void TCPChannelResourceSecure::shutdown(asio::socket_base::shutdown_type what)
{
    secure_socket_->shutdown();
}

std::string TCPChannelResourceSecure::get_password() const
{
    return parent_->configuration()->tls_config.password;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
