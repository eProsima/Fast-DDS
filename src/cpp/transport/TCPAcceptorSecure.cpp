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

void TCPAcceptorSecure::accept(
        TCPTransportInterface* parent,
        ssl::context& ssl_context)
{
    logInfo(ACEPTOR, "Listening at: " << acceptor_.local_endpoint().address()
        << ":" << acceptor_.local_endpoint().port());

    using asio::ip::tcp;
    using TLSHSRole = TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;

    // We are going to be called by the lambdas, but we may be already deleted. Save the locator so we will need it.
    Locator_t locator = locator_;
    try
    {
        acceptor_.async_accept(
            [this, locator, parent, &ssl_context](const std::error_code& error, tcp::socket socket)
            {
                if (!error)
                {
                    ssl::stream_base::handshake_type role = ssl::stream_base::server;
                    if (parent->configuration()->tls_config.handshake_role == TLSHSRole::CLIENT)
                    {
                        role = ssl::stream_base::client;
                    }

                    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> secure_socket =
                        std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(socket),ssl_context);

                    secure_socket->async_handshake(role,
                        [this, locator, parent, secure_socket](const std::error_code& error)
                        {
                            //logError(RTCP_TLS, "Handshake: " << error.message());
                            parent->SecureSocketAccepted(this, locator, error, secure_socket);
                        });
                }
                else
                {
                    parent->SecureSocketAccepted(this, locator, error, nullptr); // This method manages errors too.
                }
            });
    }
    catch(std::error_code& error)
    {
        logError(RTCP_TLS, "Exception accepting: " << error.message());
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
