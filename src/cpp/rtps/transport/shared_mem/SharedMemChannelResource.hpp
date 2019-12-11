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

#ifndef _FASTDDS_SHAREDMEM_CHANNEL_RESOURCE_INFO_
#define _FASTDDS_SHAREDMEM_CHANNEL_RESOURCE_INFO_

#include <fastdds/rtps/messages/MessageReceiver.h>
#include <fastrtps/transport/ChannelResource.h>
#include <fastrtps/rtps/common/Locator.h>

#include <rtps/transport/shared_mem/SharedMemManager.hpp>
#include <rtps/transport/shared_mem/SharedMemTransport.h>

namespace eprosima{
namespace fastdds{
namespace rtps{

class SharedMemChannelResource : public ChannelResource
{
public:

    using Log = fastdds::dds::Log;

	SharedMemChannelResource(
		SharedMemTransport* transport,
		std::shared_ptr<SharedMemManager::Listener> listener,
        uint32_t maxMsgSize,
        const fastrtps::rtps::Locator_t& locator,
        TransportReceiverInterface* receiver)
    : ChannelResource(maxMsgSize)
    , message_receiver_(receiver)
    , listener_(listener)
    , only_multicast_purpose_(false)
    , transport_(transport)
    , locator_(locator)
    {
        thread(std::thread(&SharedMemChannelResource::perform_listen_operation, this, locator));
    }

    virtual ~SharedMemChannelResource() override
    {
        message_receiver_ = nullptr;
    }

    SharedMemChannelResource& operator=(SharedMemChannelResource&& channelResource)
    {
        (void)channelResource;
        //socket_ = moveSocket(channelResource.socket_);
        return *this;
    }

    void only_multicast_purpose(const bool value)
    {
        only_multicast_purpose_ = value;
    }

    bool& only_multicast_purpose()
    {
        return only_multicast_purpose_;
    }

    bool only_multicast_purpose() const
    {
        return only_multicast_purpose_;
    }

    inline void message_receiver(TransportReceiverInterface* receiver)
    {
        message_receiver_ = receiver;
    }

    inline TransportReceiverInterface* message_receiver()
    {
        return message_receiver_;
    }

    inline virtual void disable() override
    {
        ChannelResource::disable();
    }

	const fastrtps::rtps::Locator_t& locator() const
	{
		return locator_;
	}

    void release()
    {
        listener_->close();
    }

private:

    /**
     * Function to be called from a new thread, which takes cares of performing a blocking receive
     * operation on the ReceiveResource
     * @param input_locator - Locator that triggered the creation of the resource
    */
    void perform_listen_operation(
            fastrtps::rtps::Locator_t input_locator)
    {
        fastrtps::rtps::Locator_t remote_locator;

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

    /**
    * Blocking Receive from the specified channel.
    * @param receive_buffer vector with enough capacity (not size) to accomodate a full receive buffer. That
    * capacity must not be less than the receive_buffer_size supplied to this class during construction.
    * @param receive_buffer_capacity Maximum size of the receive_buffer.
    * @param[out] receive_buffer_size Size of the received buffer.
    * @param[out] remote_locator Locator describing the remote destination we received a packet from.
    */
    bool Receive(
            fastrtps::rtps::octet* receive_buffer,
            uint32_t receive_buffer_capacity,
            uint32_t& receive_buffer_size,
            fastrtps::rtps::Locator_t& remote_locator)
    {
        (void)remote_locator;
        
        try
        {
            std::shared_ptr<SharedMemManager::Buffer> buffer = listener_->pop();

            if(buffer)
            {
                if(buffer->size() > receive_buffer_capacity)
                    throw std::runtime_error("Size of incoming message is bigger than buffer capacity");

                memcpy(receive_buffer, buffer->data(), buffer->size());
                receive_buffer_size = static_cast<uint32_t>(buffer->size());

                return (receive_buffer_size > 0);
            }
            else
            {
                return false;
            }
        }
        catch (const std::exception& error)
        {
            (void)error;
            logWarning(RTPS_MSG_OUT, "Error receiving data: " << error.what() << " - " << message_receiver()
                << " (" << this << ")");
            return false;
        }
    }

private:

    TransportReceiverInterface* message_receiver_; //Associated Readers/Writers inside of MessageReceiver
	std::shared_ptr<SharedMemManager::Listener> listener_;
    bool only_multicast_purpose_;
    SharedMemTransport* transport_;
    fastrtps::rtps::Locator_t locator_;

    SharedMemChannelResource(const SharedMemChannelResource&) = delete;
    SharedMemChannelResource& operator=(const SharedMemChannelResource&) = delete;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_CHANNEL_RESOURCE_INFO_
