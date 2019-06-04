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
#include <fastrtps/transport/UDPTransportInterface.h>
#include <fastrtps/transport/UDPChannelResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <fastrtps/utils/eClock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

UDPChannelResource::UDPChannelResource(
        UDPTransportInterface* transport,
        eProsimaUDPSocket& socket,
        uint32_t maxMsgSize,
        const Locator_t& locator,
        const std::string& sInterface,
        TransportReceiverInterface* receiver)
    : ChannelResource(maxMsgSize)
    , message_receiver_(receiver)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
    , interface_(sInterface)
    , transport_(transport)
    , closing_(false)
{
    thread(std::thread(&UDPChannelResource::perform_listen_operation, this, locator));
}

UDPChannelResource::~UDPChannelResource()
{
    message_receiver_ = nullptr;
}

void UDPChannelResource::perform_listen_operation(Locator_t input_locator)
{
    Locator_t remote_locator;

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
            logWarning(RTPS_MSG_IN, "Received Message, but no receiver attached");
        }
    }
}

bool UDPChannelResource::Receive(
        octet* receive_buffer,
        uint32_t receive_buffer_capacity,
        uint32_t& receive_buffer_size,
        Locator_t& remote_locator)
{
    try
    {
        asio::ip::udp::endpoint senderEndpoint;

        size_t bytes = socket()->receive_from(asio::buffer(receive_buffer, receive_buffer_capacity), senderEndpoint);
        receive_buffer_size = static_cast<uint32_t>(bytes);
        if (receive_buffer_size > 0)
        {
            if (receive_buffer_size == 13 && memcmp(receive_buffer, "EPRORTPSCLOSE", 13) == 0)
            {
                if (!alive())
                {
                    std::lock_guard<std::mutex> lock(mtx_closing_);
                    closing_.store(true);
                    message_receiver(nullptr);
                    cv_closing_.notify_all();
                }
                return false;
            }
            transport_->endpoint_to_locator(senderEndpoint, remote_locator);
        }
        return (receive_buffer_size > 0);
    }
    catch (const std::exception& error)
    {
        (void)error;
        logWarning(RTPS_MSG_OUT, "Error receiving data: " << error.what());
        std::cout << "+++ERROR: " << error.what() << " - " << message_receiver() << " (" << this << ")" << std::endl;
        return false;
    }
}

void UDPChannelResource::release(
        const Locator_t& locator,
        const asio::ip::address& address)
{
    if (!address.is_multicast())
    {
        std::unique_lock<std::mutex> lock(mtx_closing_);
        uint32_t tries_ = 0;
        while (!closing_.load())
        {
            transport_->ReleaseInputChannel(locator, address);
            cv_closing_.wait_for(lock, std::chrono::milliseconds(5),
                [this]{
                    return closing_.load();
                });
            ++tries_;
            if (tries_ == 10)
            {
                logError(UDPChannelResource, "After " << tries_ << " retries UDP Socket doesn't close. Aborting.");
                socket()->cancel();
                closing_.store(true);
                message_receiver(nullptr);
            }
        }
    }
    socket()->cancel();
    socket()->close();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
