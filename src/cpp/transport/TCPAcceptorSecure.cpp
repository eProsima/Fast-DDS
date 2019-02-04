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

#include <fastrtps/transport/TCPAcceptorSecure.h>
#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/utils/IPLocator.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

using namespace asio;

TCPAcceptorSecure::TCPAcceptorSecure(
        asio::io_service& io_service,
        asio::ssl::context& ssl_context,
        TCPTransportInterface* parent,
        const Locator_t& locator)
    : TCPAcceptor(io_service, parent, locator)
    , ssl_context_(ssl_context)
{
    apply_tls_config(parent->configuration());
    //secure_socket_ = tcp_secure::createTCPSocket(io_service, ssl_context);
    //set_options(parent->configuration());
}

TCPAcceptorSecure::TCPAcceptorSecure(
        asio::io_service& io_service,
        asio::ssl::context& ssl_context,
        const std::string& interface,
        const Locator_t& locator,
        const TCPTransportDescriptor* descriptor)
    : TCPAcceptor(io_service, interface, locator)
    , ssl_context_(ssl_context)
{
    apply_tls_config(descriptor);
    //secure_socket_ = tcp_secure::createTCPSocket(io_service, ssl_context);
    //set_options(descriptor);
}

void TCPAcceptorSecure::accept(
        TCPTransportInterface* parent,
        asio::io_service& io_service,
        asio::ssl::context& ssl_context)
{
    //ssl_context_ = std::move(ssl_context);
    secure_socket_ = tcp_secure::createTCPSocket(io_service, ssl_context);
    //apply_tls_config(parent->configuration());
    //set_options(parent->configuration());

    //acceptor.async_accept(secure_socket_, endpoint,
    //    std::bind(&TCPTransportInterface::SocketAccepted,
    //    parent, this, std::placeholders::_1));
    logError(ACEPTOR, "Listening at: " << acceptor_.local_endpoint().address()
        << ":" << acceptor_.local_endpoint().port());

    acceptor_.async_accept(
        [this, parent](const std::error_code& error, asio::ip::tcp::socket socket)
        {
            if (!error)
            {
                secure_socket_->async_handshake(ssl::stream_base::server,
                    [this, parent, &socket](const std::error_code& error)
                    {
                        logError(TLS_SERVER, error.message());
                        parent->SecureSocketAccepted(this, std::move(socket), error);
                    });
            }
            else
            {
                logError(RTPS_TLS, error.message());
            }

        });
}

void TCPAcceptorSecure::apply_tls_config(const TCPTransportDescriptor* descriptor)
{
    if (descriptor->apply_security)
    {
        const TCPTransportDescriptor::TLSConfig* config = &descriptor->tls_config;
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;

        tls_password_ = descriptor->tls_config.password;

        /*
        uint32_t options;
        */

        if (!config->password.empty())
        {
            ssl_context_.set_password_callback(std::bind(&TCPAcceptorSecure::get_password, this));
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
                logError(Accetor, "DEFAULT_WORKAROUNDS");
                options |= ssl::context::default_workarounds;
            }

            if (config->get_option(TLSOptions::NO_COMPRESSION))
            {
                logError(Accetor, "NO_COMPRESSION");
                options |= ssl::context::no_compression;
            }

            if (config->get_option(TLSOptions::NO_SSLV2))
            {
                logError(Accetor, "NO_SSLV2");
                options |= ssl::context::no_sslv2;
            }

            if (config->get_option(TLSOptions::NO_SSLV3))
            {
                logError(Accetor, "NO_SSLV3");
                options |= ssl::context::no_sslv3;
            }

            if (config->get_option(TLSOptions::NO_TLSV1))
            {
                logError(Accetor, "NO_TLSV1");
                options |= ssl::context::no_tlsv1;
            }

            if (config->get_option(TLSOptions::NO_TLSV1_1))
            {
                logError(Accetor, "NO_TLSV1_1");
                options |= ssl::context::no_tlsv1_1;
            }

            if (config->get_option(TLSOptions::NO_TLSV1_2))
            {
                logError(Accetor, "NO_TLSV1_2");
                options |= ssl::context::no_tlsv1_2;
            }

            // if (config->get_option(TLSOptions::NO_TLSV1_3))
            // {
            //     options |= ssl::context::no_tlsv1_3; // Asio needs to be updated
            // }

            if (config->get_option(TLSOptions::SINGLE_DH_USE))
            {
                logError(Accetor, "SINGLE_DH_USE");
                options |= ssl::context::single_dh_use;
            }

            ssl_context_.set_options(options);
        }
    }
}

void TCPAcceptorSecure::set_options(const TCPTransportDescriptor* options)
{
    //secure_socket_->lowest_layer().set_option(socket_base::receive_buffer_size(options->receiveBufferSize));
    //secure_socket_->lowest_layer().set_option(socket_base::send_buffer_size(options->sendBufferSize));
    //secure_socket_->lowest_layer().set_option(ip::tcp::no_delay(options->enable_tcp_nodelay));
    secure_socket_->set_verify_mode(ssl::verify_none);
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

std::string TCPAcceptorSecure::get_password() const
{
    return tls_password_;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
