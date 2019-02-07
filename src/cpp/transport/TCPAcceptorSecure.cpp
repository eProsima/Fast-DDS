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
        ssl::context& ssl_context,
        TCPTransportInterface* parent,
        const Locator_t& locator)
    : TCPAcceptor(io_service, parent, locator)
    , ssl_context_(ssl_context)
{
}

TCPAcceptorSecure::TCPAcceptorSecure(
        io_service& io_service,
        ssl::context& ssl_context,
        const std::string& interface,
        const Locator_t& locator)
    : TCPAcceptor(io_service, interface, locator)
    , ssl_context_(ssl_context)
{
}

void TCPAcceptorSecure::accept(
        TCPTransportInterface* parent,
        io_service& io_service,
        ssl::context& ssl_context)
{
    logInfo(ACEPTOR, "Listening at: " << acceptor_.local_endpoint().address()
        << ":" << acceptor_.local_endpoint().port());

    using asio::ip::tcp;
    try
    {
        acceptor_.async_accept(
            [this, parent, &io_service, &ssl_context](const std::error_code& error, tcp::socket socket)
            {
                tcp_secure::eProsimaTCPSocket socket_ptr = tcp_secure::createTCPSocket(std::move(socket), ssl_context);
                if (!error)
                {
                    socket_ptr->async_handshake(ssl::stream_base::server,
                        [this, parent, socket_ptr](const std::error_code& error)
                        {
                            logInfo(RTCP_TLS, "Handshake sucessfull");
                            parent->SecureSocketAccepted(this, socket_ptr, error);
                        });
                }
                else
                {
                    //logError(RTCP_TLS, "Error accepting: " << error.message());
                    //accept(parent, io_service, ssl_context);
                    parent->SecureSocketAccepted(this, socket_ptr, error); // This method manages errors too.
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
