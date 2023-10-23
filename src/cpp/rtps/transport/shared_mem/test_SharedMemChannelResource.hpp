// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_TEST_SHAREDMEM_CHANNEL_RESOURCE_
#define _FASTDDS_TEST_SHAREDMEM_CHANNEL_RESOURCE_

#include <rtps/transport/shared_mem/SharedMemChannelResource.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class test_SharedMemChannelResource : public SharedMemChannelResource
{
public:

    using Log = fastdds::dds::Log;

    test_SharedMemChannelResource(
            std::shared_ptr<SharedMemManager::Listener> listener,
            const Locator& locator,
            TransportReceiverInterface* receiver,
            uint32_t big_buffer_size,
            uint32_t* big_buffer_size_count)
        : SharedMemChannelResource(
            listener, locator, receiver,
            std::string(), ThreadSettings{},
            false, ThreadSettings{})
        , big_buffer_size_(big_buffer_size)
        , big_buffer_size_count_(big_buffer_size_count)
    {
        init_thread(locator, ThreadSettings{});
    }

    virtual ~test_SharedMemChannelResource() override
    {
    }

protected:

    uint32_t big_buffer_size_;
    uint32_t* big_buffer_size_count_;

    /**
     * Blocking Receive from the specified channel.
     */
    std::shared_ptr<SharedMemManager::Buffer> Receive(
            Locator& remote_locator) override
    {
        remote_locator.kind = LOCATOR_KIND_SHM;

        try
        {
            auto ret = listener_->pop();

            if (ret && ret->size() >= big_buffer_size_)
            {
                (*big_buffer_size_count_)++;
            }

            return ret;
        }
        catch (const std::exception& error)
        {
            (void)error;
            EPROSIMA_LOG_WARNING(RTPS_MSG_OUT, "Error receiving data: " << error.what() << " - " << message_receiver()
                                                                        << " (" << this << ")");
            return nullptr;
        }
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TEST_SHAREDMEM_CHANNEL_RESOURCE_
