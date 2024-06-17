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

#include <fastdds/utils/IPLocator.hpp>
#include <rtps/transport/TCPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Locator_t = fastdds::rtps::Locator_t;
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
        const std::string& iface,
        const Locator_t& locator)
    : TCPAcceptor(io_service, iface, locator)
{
}

void TCPAcceptorSecure::accept(
        TCPTransportInterface* parent,
        ssl::context& ssl_context)
{
    EPROSIMA_LOG_INFO(ACEPTOR, "Listening at: " << acceptor_.local_endpoint().address()
                                                << ":" << acceptor_.local_endpoint().port());

    using asio::ip::tcp;
    using TLSHSRole = TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;
    const Locator_t locator = locator_;

    try
    {
#if ASIO_VERSION >= 101200
        acceptor_.async_accept(
            [locator, parent, &ssl_context](const std::error_code& error, tcp::socket socket)
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

                    secure_socket->async_handshake(role,
                    [secure_socket, locator, parent](const std::error_code& error)
                    {
                        //EPROSIMA_LOG_ERROR(RTCP_TLS, "Handshake: " << error.message());
                        parent->SecureSocketAccepted(secure_socket, locator, error);
                    });
                }
                else
                {
                    parent->SecureSocketAccepted(nullptr, locator, error); // This method manages errors too.
                }
            });
#else
        auto secure_socket = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(*io_service_, ssl_context);

        acceptor_.async_accept(secure_socket->lowest_layer(),
                [locator, parent, secure_socket](const std::error_code& error)
                {
                    if (!error)
                    {
                        ssl::stream_base::handshake_type role = ssl::stream_base::server;
                        if (parent->configuration()->tls_config.handshake_role == TLSHSRole::CLIENT)
                        {
                            role = ssl::stream_base::client;
                        }

                        secure_socket->async_handshake(role,
                        [secure_socket, locator, parent](const std::error_code& error)
                        {
                            //EPROSIMA_LOG_ERROR(RTCP_TLS, "Handshake: " << error.message());
                            parent->SecureSocketAccepted(secure_socket, locator, error);
                        });
                    }
                    else
                    {
                        parent->SecureSocketAccepted(nullptr, locator, error); // This method manages errors too.
                    }
                });
#endif // if ASIO_VERSION >= 101200
    }
    catch (std::error_code& error)
    {
        EPROSIMA_LOG_ERROR(RTCP_TLS, "Exception accepting: " << error.message());
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
