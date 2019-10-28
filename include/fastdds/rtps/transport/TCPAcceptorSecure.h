// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_TCP_ACCEPTOR_SECURE_
#define _FASTDDS_TCP_ACCEPTOR_SECURE_

#include <fastdds/rtps/transport/TCPAcceptor.h>
#include <fastdds/rtps/transport/TCPChannelResourceSecure.h>
#include <asio/ssl.hpp>

namespace eprosima{
namespace fastdds{
namespace rtps{

/**
 * TLS TCP Socket acceptor wrapper class.
 */
class TCPAcceptorSecure : public TCPAcceptor
{
public:
    /**
    * Constructor
    * @param io_service Reference to the ASIO service.
    * @param parent Pointer to the transport that is going to manage the acceptor.
    * @param locator Locator with the information about where to accept connections.
    */
    TCPAcceptorSecure(
        asio::io_service& io_service,
        TCPTransportInterface* parent,
        const fastrtps::rtps::Locator_t& locator);

    /**
    * Constructor
    * @param io_service Reference to the ASIO service.
    * @param interface Network interface to bind the socket
    * @param locator Locator with the information about where to accept connections.
    */
    TCPAcceptorSecure(
        asio::io_service& io_service,
        const std::string& interface,
        const fastrtps::rtps::Locator_t& locator);

    /**
    * Destructor
    */
    virtual ~TCPAcceptorSecure()
    {
        acceptor_.cancel();
        acceptor_.close();
    }

    //! Method to start the accepting process.
    void accept(
        TCPTransportInterface* parent,
        asio::ssl::context&);

};


} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TCP_ACCEPTOR_SECURE_
