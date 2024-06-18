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

#include <rtps/transport/UDPChannelResource.h>

#include <asio.hpp>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

#include <rtps/messages/MessageReceiver.h>
#include <rtps/transport/UDPTransportInterface.h>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

UDPChannelResource::UDPChannelResource(
        UDPTransportInterface* transport,
        eProsimaUDPSocket& socket,
        uint32_t maxMsgSize,
        const Locator& locator,
        const std::string& sInterface,
        TransportReceiverInterface* receiver,
        const ThreadSettings& thread_config)
    : ChannelResource(maxMsgSize)
    , message_receiver_(receiver)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
    , interface_(sInterface)
    , transport_(transport)
{
    auto fn = [this, locator]()
            {
                perform_listen_operation(locator);
            };
    thread(create_thread(fn, thread_config, "dds.udp.%u", locator.port));
}

UDPChannelResource::~UDPChannelResource()
{
    message_receiver_ = nullptr;

    asio::error_code ec;
    socket()->close(ec);
}

void UDPChannelResource::perform_listen_operation(
        Locator input_locator)
{
    Locator remote_locator;

    while (alive())
    {
        // Blocking receive.
        auto& msg = message_buffer();
        if (!Receive(msg.buffer, msg.max_size, msg.length, remote_locator))
        {
            continue;
        }

        // Processes the data through the CDR Message interface.
        if (message_receiver() != nullptr)
        {
            message_receiver()->OnDataReceived(msg.buffer, msg.length, input_locator, remote_locator);
        }
        else if (alive())
        {
            EPROSIMA_LOG_WARNING(RTPS_MSG_IN, "Received Message, but no receiver attached");
        }
    }

    message_receiver(nullptr);
}

bool UDPChannelResource::Receive(
        octet* receive_buffer,
        uint32_t receive_buffer_capacity,
        uint32_t& receive_buffer_size,
        Locator& remote_locator)
{
    try
    {
        asio::ip::udp::endpoint senderEndpoint;

        size_t bytes = socket()->receive_from(asio::buffer(receive_buffer, receive_buffer_capacity), senderEndpoint);
        receive_buffer_size = static_cast<uint32_t>(bytes);
        if (receive_buffer_size > 0)
        {
            // This is not necessary anymore but it's left here for back compatibility with versions older than 1.8.1
            if (receive_buffer_size == 13 && memcmp(receive_buffer, "EPRORTPSCLOSE", 13) == 0)
            {
                return false;
            }
            transport_->endpoint_to_locator(senderEndpoint, remote_locator);
        }
        return (receive_buffer_size > 0);
    }
    catch (const std::exception& error)
    {
        (void)error;
        EPROSIMA_LOG_WARNING(RTPS_MSG_OUT, "Error receiving data: " << error.what() << " - " << message_receiver()
                                                                    << " (" << this << ")");
        return false;
    }
}

void UDPChannelResource::release()
{
    // Cancel all asynchronous operations associated with the socket.
    socket()->cancel();
    // Disable receives on the socket.
    // shutdown always returns a 'shutdown: Transport endpoint is not connected' error,
    // since the endpoint is indeed not connected. However, it unblocks the synchronous receive
    // in Windows and Linux anyways, which is what we want.
    asio::error_code ec;
    socket()->shutdown(asio::socket_base::shutdown_type::shutdown_receive, ec);

#if defined(__APPLE__)
    // On OSX shutdown does not seem to unblock the listening thread, but close does.
    socket()->close();
#endif // if defined(__APPLE__)
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
