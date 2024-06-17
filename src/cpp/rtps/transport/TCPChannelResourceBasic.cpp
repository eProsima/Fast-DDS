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

#include <rtps/transport/TCPChannelResourceBasic.h>

#include <future>
#include <array>

#include <asio.hpp>
#include <fastdds/utils/IPLocator.hpp>
#include <rtps/transport/TCPTransportInterface.h>

using namespace asio;

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

TCPChannelResourceBasic::TCPChannelResourceBasic(
        TCPTransportInterface* parent,
        asio::io_service& service,
        const Locator& locator,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, locator, maxMsgSize)
    , service_(service)
{
}

TCPChannelResourceBasic::TCPChannelResourceBasic(
        TCPTransportInterface* parent,
        asio::io_service& service,
        std::shared_ptr<asio::ip::tcp::socket> socket,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, maxMsgSize)
    , service_(service)
    , socket_(socket)
{
}

TCPChannelResourceBasic::~TCPChannelResourceBasic()
{
}

void TCPChannelResourceBasic::connect(
        const std::shared_ptr<TCPChannelResource>& myself)
{
    assert(TCPConnectionType::TCP_CONNECT_TYPE == tcp_connection_type_);
    eConnectionStatus expected = eConnectionStatus::eDisconnected;

    if (connection_status_.compare_exchange_strong(expected, eConnectionStatus::eConnecting))
    {
        try
        {
            ip::tcp::resolver resolver(service_);

            auto endpoints = resolver.resolve({
                            IPLocator::hasWan(locator_) ? IPLocator::toWanstring(locator_) : IPLocator::ip_to_string(
                                locator_),
                            std::to_string(IPLocator::getPhysicalPort(locator_))});

            socket_ = std::make_shared<asio::ip::tcp::socket>(service_);
            std::weak_ptr<TCPChannelResource> channel_weak_ptr = myself;

            asio::async_connect(
                *socket_,
                endpoints,
                [this, channel_weak_ptr](std::error_code ec
#if ASIO_VERSION >= 101200
                , ip::tcp::endpoint
#else
                , ip::tcp::resolver::iterator
#endif // if ASIO_VERSION >= 101200
                )
                {
                    if (!channel_weak_ptr.expired())
                    {
                        parent_->SocketConnected(channel_weak_ptr, ec);
                    }
                }
                );
        }
        catch (const std::system_error& error)
        {
            EPROSIMA_LOG_ERROR(RTCP, "Openning socket " << error.what());
        }
    }
}

void TCPChannelResourceBasic::disconnect()
{
    if (eConnecting < change_status(eConnectionStatus::eDisconnected) && alive())
    {
        std::lock_guard<std::mutex> read_lock(read_mutex_);
        auto socket = socket_;

        std::error_code ec;
        socket->shutdown(asio::ip::tcp::socket::shutdown_both, ec);

        service_.post([&, socket]()
                {
                    try
                    {
                        socket->cancel();
                        socket->close();
                    }
                    catch (std::exception&)
                    {
                    }
                });
    }
}

uint32_t TCPChannelResourceBasic::read(
        octet* buffer,
        std::size_t size,
        asio::error_code& ec)
{
    std::unique_lock<std::mutex> read_lock(read_mutex_);

    if (eConnecting < connection_status_)
    {
        return static_cast<uint32_t>(asio::read(*socket_, asio::buffer(buffer, size), transfer_exactly(size), ec));
    }

    return 0;
}

size_t TCPChannelResourceBasic::send(
        const octet* header,
        size_t header_size,
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        asio::error_code& ec)
{
    size_t bytes_sent = 0;

    if (eConnecting < connection_status_)
    {
        std::lock_guard<std::mutex> send_guard(send_mutex_);

        if (parent_->configuration()->non_blocking_send &&
                !check_socket_send_buffer(header_size + total_bytes, socket_->native_handle()))
        {
            return 0;
        }

        // Use a list of const_buffers to send the message
        std::list<asio::const_buffer> asio_buffers;
        if (header_size > 0)
        {
            asio_buffers.push_back(asio::buffer(header, header_size));
        }
        asio_buffers.insert(asio_buffers.end(), buffers.begin(), buffers.end());
        bytes_sent = asio::write(*socket_.get(), asio_buffers, ec);
    }

    return bytes_sent;
}

asio::ip::tcp::endpoint TCPChannelResourceBasic::remote_endpoint() const
{
    return socket_->remote_endpoint();
}

asio::ip::tcp::endpoint TCPChannelResourceBasic::local_endpoint() const
{
    return socket_->local_endpoint();
}

asio::ip::tcp::endpoint TCPChannelResourceBasic::remote_endpoint(
        asio::error_code& ec) const
{
    return socket_->remote_endpoint(ec);
}

asio::ip::tcp::endpoint TCPChannelResourceBasic::local_endpoint(
        asio::error_code& ec) const
{
    return socket_->local_endpoint(ec);
}

void TCPChannelResourceBasic::set_options(
        const TCPTransportDescriptor* options)
{
    TCPChannelResource::set_socket_options(*socket_, options);
}

void TCPChannelResourceBasic::cancel()
{
    socket_->cancel();
}

void TCPChannelResourceBasic::close()
{
    socket_->close();
}

void TCPChannelResourceBasic::shutdown(
        asio::socket_base::shutdown_type what)
{
    socket_->shutdown(what);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
