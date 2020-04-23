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

#ifndef _FASTDDS_SHAREDMEM_CHANNEL_RESOURCE_
#define _FASTDDS_SHAREDMEM_CHANNEL_RESOURCE_

#include <fastdds/rtps/messages/MessageReceiver.h>
#include <fastrtps/transport/ChannelResource.h>
#include <fastrtps/rtps/common/Locator.h>

#include <rtps/transport/shared_mem/SharedMemManager.hpp>
#include <rtps/transport/shared_mem/SharedMemTransport.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class SharedMemChannelResource : public ChannelResource
{
public:

    using Log = fastdds::dds::Log;

    SharedMemChannelResource(
            std::shared_ptr<SharedMemManager::Listener> listener,
            const fastrtps::rtps::Locator_t& locator,
            TransportReceiverInterface* receiver,
            const std::string& dump_file,
            bool should_init_thread = true)
        : ChannelResource()
        , message_receiver_(receiver)
        , listener_(listener)
        , only_multicast_purpose_(false)
        , locator_(locator)
    {
        if (!dump_file.empty())
        {
            auto packets_file_consumer = std::unique_ptr<SHMPacketFileConsumer>(
                new SHMPacketFileConsumer(dump_file));

            packet_logger_ = std::make_shared<PacketsLog<SHMPacketFileConsumer> >();
            packet_logger_->RegisterConsumer(std::move(packets_file_consumer));
        }

        if (should_init_thread)
        {
            init_thread(locator);
        }
    }

    virtual ~SharedMemChannelResource() override
    {
        message_receiver_ = nullptr;
    }

    void only_multicast_purpose(
            const bool value)
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

    inline void message_receiver(
            TransportReceiverInterface* receiver)
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
        try
        {
            listener_->close();
        }
        catch(const std::exception& e)
        {
            logWarning(RTPS_MSG_IN, e.what());
        }
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
            std::shared_ptr<SharedMemManager::Buffer> message;

            if (!(message = Receive(remote_locator)) )
            {
                continue;
            }

            if (packet_logger_)
            {
                packet_logger_->QueueLog({packet_logger_->now(), input_locator, remote_locator, message});
            }

            // Processes the data through the CDR Message interface.
            if (message_receiver() != nullptr)
            {
                message_receiver()->OnDataReceived(
                    static_cast<fastrtps::rtps::octet*>(message->data()),
                    message->size(),
                    input_locator, remote_locator);
            }
            else if (alive())
            {
                logWarning(RTPS_MSG_IN, "Received Message, but no receiver attached");
            }

            // Forces message release before waiting for the next
            message.reset();
        }

        message_receiver(nullptr);
    }

protected:

    void init_thread(
            const fastrtps::rtps::Locator_t& locator)
    {
        this->thread(std::thread(&SharedMemChannelResource::perform_listen_operation, this, locator));
    }


    /**
     * Blocking Receive from the specified channel.
     */
    virtual std::shared_ptr<SharedMemManager::Buffer> Receive(
            fastrtps::rtps::Locator_t& remote_locator)
    {
        (void)remote_locator;

        try
        {
            return listener_->pop();
        }
        catch (const std::exception& error)
        {
            (void)error;
            logWarning(RTPS_MSG_OUT, "Error receiving data: " << error.what() << " - " << message_receiver()
                                                              << " (" << this << ")");
            return nullptr;
        }
    }

private:

    TransportReceiverInterface* message_receiver_; //Associated Readers/Writers inside of MessageReceiver

    // Allows dumping of received packets to a file
    std::shared_ptr<PacketsLog<SHMPacketFileConsumer>> packet_logger_;

protected:

    std::shared_ptr<SharedMemManager::Listener> listener_;

private:

    bool only_multicast_purpose_;
    fastrtps::rtps::Locator_t locator_;

    SharedMemChannelResource(
            const SharedMemChannelResource&) = delete;
    SharedMemChannelResource& operator=(
            const SharedMemChannelResource&) = delete;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_SHAREDMEM_CHANNEL_RESOURCE_
