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

#ifndef _FASTDDS_TRANSPORT_UDPSENDERRESOURCE_HPP_
#define _FASTDDS_TRANSPORT_UDPSENDERRESOURCE_HPP_

#include <fastrtps/rtps/network/SenderResource.h>

#include <rtps/transport/shared_mem/SharedMemTransport.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class SharedMemSenderResource : public fastrtps::rtps::SenderResource
{
public:

    SharedMemSenderResource(
            SharedMemTransport& transport,
            std::shared_ptr<SharedMemManager> shared_mem_manager,
            bool only_multicast_purpose = false)
        : fastrtps::rtps::SenderResource(transport.kind())
        , shared_mem_manager_(shared_mem_manager)
        , only_multicast_purpose_(only_multicast_purpose)
    {
        // Implementation functions are bound to the right transport parameters
        /*clean_up = [this, &transport]()
            {
                transport.CloseOutputChannel(socket_);
            };*/

        send_lambda_ = [this, &transport] (
                const fastrtps::rtps::octet* data,
                uint32_t dataSize,
                fastrtps::rtps::LocatorsIterator* destination_locators_begin,
                fastrtps::rtps::LocatorsIterator* destination_locators_end,
                const std::chrono::steady_clock::time_point& max_blocking_time_point)-> bool
            {
                return transport.send(data, dataSize, /*writter_*/nullptr, destination_locators_begin, destination_locators_end, only_multicast_purpose_, max_blocking_time_point);
            };
    }

    virtual ~SharedMemSenderResource()
    {
        if (clean_up)
        {
            clean_up();
        }
    }

    static SharedMemSenderResource* cast(TransportInterface& transport, SenderResource* sender_resource)
    {
        SharedMemSenderResource* returned_resource = nullptr;

        if (sender_resource->kind() == transport.kind())
        {
            returned_resource = dynamic_cast<SharedMemSenderResource*>(sender_resource);
        }

        return returned_resource;
    }

private:

    SharedMemSenderResource() = delete;

    SharedMemSenderResource(const SenderResource&) = delete;

    SharedMemSenderResource& operator=(const SenderResource&) = delete;

    std::shared_ptr<SharedMemManager> shared_mem_manager_;

    bool only_multicast_purpose_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_TRANSPORT_UDPSENDERRESOURCE_HPP_
