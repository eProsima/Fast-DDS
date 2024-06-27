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

#include <rtps/transport/TCPAcceptorBasic.h>

#include <fastdds/utils/IPLocator.hpp>
#include <rtps/transport/TCPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using IPLocator = fastdds::rtps::IPLocator;

TCPAcceptorBasic::TCPAcceptorBasic(
        asio::io_service& io_service,
        TCPTransportInterface* parent,
        const Locator& locator)
    : TCPAcceptor(io_service, parent, locator)
    , socket_(*io_service_)
{
    endpoint_ = asio::ip::tcp::endpoint(parent->generate_protocol(), IPLocator::getPhysicalPort(locator_));
}

TCPAcceptorBasic::TCPAcceptorBasic(
        asio::io_service& io_service,
        const std::string& iface,
        const Locator& locator)
    : TCPAcceptor(io_service, iface, locator)
    , socket_(*io_service_)
{
    endpoint_ = asio::ip::tcp::endpoint(asio::ip::address::from_string(iface),
                    IPLocator::getPhysicalPort(locator_));
}

void TCPAcceptorBasic::accept(
        TCPTransportInterface* parent)
{
    using asio::ip::tcp;

    const Locator locator = locator_;

    acceptor_.async_accept(socket_,
            [parent, locator, this](const std::error_code& error)
            {
                if (!error)
                {
                    auto socket = std::make_shared<tcp::socket>(std::move(socket_));
                    parent->SocketAccepted(socket, locator, error);
                }
            });
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
