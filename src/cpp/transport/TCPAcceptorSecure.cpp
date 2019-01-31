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

TCPAcceptorSecure::TCPAcceptorSecure(
    asio::io_service& io_service,
    asio::ssl::context& ssl_context,
    TCPTransportInterface* parent,
    const Locator_t& locator)
    : TCPAcceptor(io_service, parent, locator)
{
    secure_socket_ = tcp_secure::createTCPSocket(io_service, ssl_context);
}

TCPAcceptorSecure::TCPAcceptorSecure(
    asio::io_service& io_service,
    asio::ssl::context& ssl_context,
    const std::string& interface,
    const Locator_t& locator)
    : TCPAcceptor(io_service, interface, locator)
{
    secure_socket_ = tcp_secure::createTCPSocket(io_service, ssl_context);
}

void TCPAcceptorSecure::Accept(
    TCPTransportInterface* parent,
    asio::io_service& io_service,
    asio::ssl::context& ssl_context)
{
    secure_socket_ = tcp_secure::createTCPSocket(io_service, ssl_context);

    //acceptor.async_accept(secure_socket_, endpoint,
    //    std::bind(&TCPTransportInterface::SocketAccepted,
    //    parent, this, std::placeholders::_1));
    acceptor.async_accept(
        [this, parent](const std::error_code& error, asio::ip::tcp::socket socket)
        {
            parent->SecureSocketAccepted(this, std::move(socket), error);
        });
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
