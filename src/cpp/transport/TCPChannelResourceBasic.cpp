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

#include <asio.hpp>
#include <fastrtps/transport/TCPChannelResource.h>
#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/eClock.h>

using namespace asio;

namespace eprosima {
namespace fastrtps {
namespace rtps {

TCPChannelResourceBasic::TCPChannelResourceBasic(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        asio::io_service& service,
        const Locator_t& locator,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, rtcpManager, locator, maxMsgSize)
    , service_(service)
    , socket_(std::make_shared<asio::ip::tcp::socket>(service))
{
}

TCPChannelResourceBasic::TCPChannelResourceBasic(
        TCPTransportInterface* parent,
        RTCPMessageManager* rtcpManager,
        asio::io_service& service,
        std::shared_ptr<asio::ip::tcp::socket> socket,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, rtcpManager, maxMsgSize)
    , service_(service)
    , socket_(socket)
{
}

TCPChannelResourceBasic::~TCPChannelResourceBasic()
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

void TCPChannelResourceBasic::connect()
{
    std::unique_lock<std::mutex> scoped(status_mutex_);
    if (connection_status_ == eConnectionStatus::eDisconnected)
    {
        connection_status_ = eConnectionStatus::eConnecting;
        //ip::tcp::endpoint endpoint = parent_->generate_local_endpoint(locator_, IPLocator::getPhysicalPort(locator_));
        //logError(DEBUG, "Connect to endpoint: " << endpoint.address() << ":" << endpoint.port()
        //    << " from locator " << IPLocator::to_string(locator_));
        try
        {
            //socket_->async_connect(endpoint, std::bind(&TCPTransportInterface::SocketConnected, parent_,
            //    locator_, std::placeholders::_1));
            Locator_t locator = locator_;

            ip::tcp::resolver resolver(service_);

            auto endpoints = resolver.resolve(
                IPLocator::hasWan(locator_) ? IPLocator::toWanstring(locator_) : IPLocator::ip_to_string(locator_),
                std::to_string(IPLocator::getPhysicalPort(locator_)));

            asio::async_connect(
                *socket_,
                endpoints,
                [this, locator](std::error_code ec, ip::tcp::endpoint)
                {
                    parent_->SocketConnected(locator, ec);
                }
            );
        }
        catch(const std::system_error &error)
        {
            logError(RTCP, "Openning socket " << error.what());
        }
    }
}

void TCPChannelResourceBasic::disconnect()
{
    if (change_status(eConnectionStatus::eDisconnected))
    {
        try
        {
            asio::error_code ec;
            socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            socket_->cancel();

          // This method was added on the version 1.12.0
#if ASIO_VERSION >= 101200 && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0603)
            socket_->release();
#endif
        }
        catch (std::exception&)
        {
            // Cancel & shutdown throws exceptions if the socket has been closed ( Test_TCPv4Transport )
        }
        socket_->close();
    }
}

uint32_t TCPChannelResourceBasic::read(
        octet* buffer,
        std::size_t size,
        asio::error_code& ec)
{
    std::unique_lock<std::recursive_mutex> read_lock(read_mutex());
    //return static_cast<uint32_t>(asio::read(*socket_, asio::buffer(buffer, buffer_capacity),
    //    transfer_exactly(size), ec));
    return static_cast<uint32_t>(asio::read(*socket_, asio::buffer(buffer, size), transfer_exactly(size), ec));
/*
    size_t rec = 0;
    size_t readed = static_cast<size_t>(-1);

    while (readed != 0 && rec < size && !ec && alive())
    {
        readed = socket_->read_some(asio::buffer(buffer + rec, size - rec), ec);
        rec += readed;
    }
    return static_cast<uint32_t>(rec);
*/
}

uint32_t TCPChannelResourceBasic::send(
        const octet* data,
        size_t size,
        asio::error_code& ec)
{
    std::unique_lock<std::recursive_mutex> write_lock(write_mutex());
    //return static_cast<uint32_t>(socket_->write_some(asio::buffer(data, size), ec));
    return static_cast<uint32_t>(socket_->send(asio::buffer(data, size), 0, ec));
/*
    size_t sent = 0;
    size_t aux = static_cast<size_t>(-1);

    while (aux != 0 && sent < size && !ec && alive())
    {
        aux = socket_->write_some(asio::buffer(data + sent, size - sent), ec);
        sent += aux;
    }

    return static_cast<uint32_t>(sent);
*/
}

asio::ip::tcp::endpoint TCPChannelResourceBasic::remote_endpoint() const
{
    return socket_->remote_endpoint();
}

asio::ip::tcp::endpoint TCPChannelResourceBasic::local_endpoint() const
{
    return socket_->local_endpoint();
}

void TCPChannelResourceBasic::set_options(const TCPTransportDescriptor* options)
{
    socket_->set_option(socket_base::receive_buffer_size(options->receiveBufferSize));
    socket_->set_option(socket_base::send_buffer_size(options->sendBufferSize));
    socket_->set_option(ip::tcp::no_delay(options->enable_tcp_nodelay));
}

void TCPChannelResourceBasic::cancel()
{
    socket_->cancel();
}

void TCPChannelResourceBasic::close()
{
    socket_->close();
}

void TCPChannelResourceBasic::shutdown(asio::socket_base::shutdown_type what)
{
    socket_->shutdown(what);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
