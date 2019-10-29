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

#include <asio.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportInterface.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemChannelResource.h>
#include <fastdds/rtps/messages/MessageReceiver.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Locator_t = fastrtps::rtps::Locator_t;
using octet = fastrtps::rtps::octet;

SharedMemChannelResource::SharedMemChannelResource(
		SharedMemTransportInterface* transport,
		std::shared_ptr<eProsimaSharedMem::Reader> reader,
        uint32_t maxMsgSize,
        const Locator_t& locator,
        TransportReceiverInterface* receiver)
    : ChannelResource(maxMsgSize)
    , message_receiver_(receiver)
    , reader_(reader)
    , only_multicast_purpose_(false)
    , transport_(transport)
{
    thread(std::thread(&SharedMemChannelResource::perform_listen_operation, this, locator));
}

SharedMemChannelResource::~SharedMemChannelResource()
{
    message_receiver_ = nullptr;
}

void SharedMemChannelResource::perform_listen_operation(Locator_t input_locator)
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

    message_receiver(nullptr);
}

bool SharedMemChannelResource::Receive(
        octet* receive_buffer,
        uint32_t receive_buffer_capacity,
        uint32_t& receive_buffer_size,
        Locator_t& remote_locator)
{
    try
    {
        size_t bytes = reader_->read(receive_buffer, receive_buffer_capacity);
        receive_buffer_size = static_cast<uint32_t>(bytes);

        return (receive_buffer_size > 0);
    }
    catch (const std::exception& error)
    {
        (void)error;
        logWarning(RTPS_MSG_OUT, "Error receiving data: " << error.what() << " - " << message_receiver()
            << " (" << this << ")");
        return false;
    }
}

void SharedMemChannelResource::release()
{
    // Close and cancel all asynchronous operations associated with the socket.
	reader_.reset();
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
