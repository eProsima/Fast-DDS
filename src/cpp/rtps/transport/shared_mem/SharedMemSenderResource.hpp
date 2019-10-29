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

#ifndef __TRANSPORT_UDPSENDERRESOURCE_HPP__
#define __TRANSPORT_UDPSENDERRESOURCE_HPP__

#include <fastrtps/rtps/network/SenderResource.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportInterface.h>
#include <fastdds/rtps/transport/shared_mem/eProsimaSharedMem.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class SharedMemSenderResource : public fastrtps::rtps::SenderResource
{
    public:

        SharedMemSenderResource(
                SharedMemTransportInterface& transport,
				std::shared_ptr<eProsimaSharedMem::Writter> writter,
                bool only_multicast_purpose = false)
            : fastrtps::rtps::SenderResource(transport.kind())
            , writter_(writter)
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
                    const fastrtps::rtps::Locator_t& destination,
                    const std::chrono::microseconds& timeout)-> bool
                {
                    return transport.send(data, dataSize, writter_, destination, only_multicast_purpose_, timeout);
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

		std::shared_ptr<eProsimaSharedMem::Writter> writter_;

        bool only_multicast_purpose_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // __TRANSPORT_SHAREDMEMSENDERRESOURCE_HPP__
