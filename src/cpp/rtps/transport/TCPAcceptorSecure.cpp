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

#include <rtps/transport/TCPAcceptorSecure.h>

#include <chrono>

#include <asio/steady_timer.hpp>
#include <fastrtps/utils/IPLocator.h>
#include <rtps/transport/TCPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Locator_t = fastrtps::rtps::Locator_t;
using Log = fastdds::dds::Log;

using namespace asio;

TCPAcceptorSecure::TCPAcceptorSecure(
        io_service& io_service,
        TCPTransportInterface* parent,
        const Locator_t& locator)
    : TCPAcceptor(io_service, parent, locator)
{
}

TCPAcceptorSecure::TCPAcceptorSecure(
        io_service& io_service,
        const std::string& interface,
        const Locator_t& locator)
    : TCPAcceptor(io_service, interface, locator)
{
}

constexpr uint32_t TCPAcceptorSecure::handshake_timeout_ms;

void TCPAcceptorSecure::accept(
        TCPTransportInterface* parent,
        ssl::context& ssl_context)
{
    logInfo(ACEPTOR, "Listening at: " << acceptor_.local_endpoint().address()
                                      << ":" << acceptor_.local_endpoint().port());

    using asio::ip::tcp;
    using TLSHSRole = TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;
    const Locator_t locator = locator_;
    io_service* context = io_service_;

    try
    {
#if ASIO_VERSION >= 101200
        acceptor_.async_accept(
            [locator, parent, &ssl_context, context](const std::error_code& error, tcp::socket socket)
            {
                if (!error)
                {
                    ssl::stream_base::handshake_type role = ssl::stream_base::server;
                    if (parent->configuration()->tls_config.handshake_role == TLSHSRole::CLIENT)
                    {
                        role = ssl::stream_base::client;
                    }

                    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> secure_socket =
                    std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(socket), ssl_context);

                    // Close the socket if the handshake does not finish in time. This aborts the pending handshake.
                    std::shared_ptr<asio::steady_timer> handshake_timer =
                    std::make_shared<asio::steady_timer>(*context);
                    handshake_timer->expires_after(std::chrono::milliseconds(handshake_timeout_ms));
                    handshake_timer->async_wait([secure_socket](const std::error_code& timer_error)
                    {
                        if (asio::error::operation_aborted != timer_error)
                        {
                            logWarning(RTCP_TLS, "TLS handshake timeout. Closing the connection.");
                            std::error_code ec;
                            secure_socket->lowest_layer().close(ec);
                        }
                    });

                    secure_socket->async_handshake(role,
                    [secure_socket, handshake_timer, parent](const std::error_code& handshake_error)
                    {
                        handshake_timer->cancel();
                        std::error_code result = handshake_error;
                        if (!result && !secure_socket->lowest_layer().is_open())
                        {
                            // Timeout closed the socket right after the handshake finished
                            result = asio::error::timed_out;
                        }
                        parent->SecureSocketHandshakeCompleted(secure_socket, result);
                    });
                }

                // Accept next connection without waiting for the handshake to finish
                parent->SecureSocketAccepted(locator, error);
            });
#else
        auto secure_socket = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(*io_service_, ssl_context);

        acceptor_.async_accept(secure_socket->lowest_layer(),
                [locator, parent, secure_socket, context](const std::error_code& error)
                {
                    if (!error)
                    {
                        ssl::stream_base::handshake_type role = ssl::stream_base::server;
                        if (parent->configuration()->tls_config.handshake_role == TLSHSRole::CLIENT)
                        {
                            role = ssl::stream_base::client;
                        }

                        // Close the socket if the handshake does not finish in time. This aborts the pending handshake.
                        std::shared_ptr<asio::steady_timer> handshake_timer =
                        std::make_shared<asio::steady_timer>(*context);
                        handshake_timer->expires_from_now(std::chrono::milliseconds(handshake_timeout_ms));
                        handshake_timer->async_wait([secure_socket](const std::error_code& timer_error)
                        {
                            if (asio::error::operation_aborted != timer_error)
                            {
                                logWarning(RTCP_TLS, "TLS handshake timeout. Closing the connection.");
                                std::error_code ec;
                                secure_socket->lowest_layer().close(ec);
                            }
                        });

                        secure_socket->async_handshake(role,
                        [secure_socket, handshake_timer, parent](const std::error_code& handshake_error)
                        {
                            handshake_timer->cancel();
                            std::error_code result = handshake_error;
                            if (!result && !secure_socket->lowest_layer().is_open())
                            {
                                // Timeout closed the socket right after the handshake finished
                                result = asio::error::timed_out;
                            }
                            parent->SecureSocketHandshakeCompleted(secure_socket, result);
                        });
                    }

                    // Accept next connection without waiting for the handshake to finish
                    parent->SecureSocketAccepted(locator, error);
                });
#endif // if ASIO_VERSION >= 101200
    }
    catch (std::error_code& error)
    {
        logError(RTCP_TLS, "Exception accepting: " << error.message());
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
