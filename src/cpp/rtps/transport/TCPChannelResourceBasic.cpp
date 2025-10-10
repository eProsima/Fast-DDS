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

#include <array>
#include <future>
#include <mutex>

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
        asio::io_context& context,
        const Locator& locator,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, locator, maxMsgSize)
    , context_(context)
{
}

TCPChannelResourceBasic::TCPChannelResourceBasic(
        TCPTransportInterface* parent,
        asio::io_context& context,
        std::shared_ptr<asio::ip::tcp::socket> socket,
        uint32_t maxMsgSize)
    : TCPChannelResource(parent, maxMsgSize)
    , context_(context)
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
            ip::tcp::resolver resolver(context_);

            auto endpoints = resolver.resolve(
                IPLocator::hasWan(locator_) ? IPLocator::toWanstring(locator_) : IPLocator::ip_to_string(
                    locator_),
                std::to_string(IPLocator::getPhysicalPort(locator_)));

            socket_ = std::make_shared<asio::ip::tcp::socket>(context_);
            std::weak_ptr<TCPChannelResource> channel_weak_ptr = myself;

            asio::async_connect(
                *socket_,
                endpoints,
                [this, channel_weak_ptr](std::error_code ec, ip::tcp::endpoint)
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
    // Go to disconnecting state to protect from concurrent connects and disconnects
    auto prev_status = change_status(eConnectionStatus::eDisconnecting);
    if (eConnecting < prev_status && alive())
    {
        // Shutdown the socket to abort any ongoing read and write operations
        shutdown(asio::ip::tcp::socket::shutdown_both);

        cancel();
        close(); // Blocks until all read and write operations have finished

        // Change to disconnected state as the last step
        change_status(eConnectionStatus::eDisconnected);
    }
    else if (eConnectionStatus::eDisconnecting != prev_status || !alive())
    {
        change_status(eConnectionStatus::eDisconnected);
    }
}

uint32_t TCPChannelResourceBasic::read(
        octet* buffer,
        std::size_t size,
        asio::error_code& ec)
{
    if (connected())
    {
        std::unique_lock<std::mutex> read_lock(read_mutex_);
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

    if (connected())
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
    set_socket_options(*socket_, options);
}

void TCPChannelResourceBasic::cancel()
{
    std::error_code ec;
    socket_->cancel(ec); // thread safe with respect to asio's read and write methods
}

void TCPChannelResourceBasic::close()
{
    // Wait for read and write operations to finish before closing the socket (otherwise not thread safe)
    // NOTE: shutdown should have been called before closing to abort any ongoing operation
    std::unique_lock<std::mutex> send_lk(send_mutex_, std::defer_lock);
    std::unique_lock<std::mutex> read_lk(read_mutex_, std::defer_lock);
    std::lock(send_lk, read_lk); // Pre C++17 alternative to std::scoped_lock

    std::error_code ec;
    socket_->close(ec);
}

void TCPChannelResourceBasic::shutdown(
        asio::socket_base::shutdown_type what)
{
    std::error_code ec;
    socket_->shutdown(what, ec); // thread safe with respect to asio's read and write methods
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
