// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/network/SenderResource.h>
#include <fastdds/rtps/transport/UDPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class UDPSenderResource : public fastrtps::rtps::SenderResource
{
    public:

        UDPSenderResource(
                UDPTransportInterface& transport,
                eProsimaUDPSocket& socket,
                bool only_multicast_purpose = false)
            : SenderResource(transport.kind())
            , socket_(moveSocket(socket))
            , only_multicast_purpose_(only_multicast_purpose)
        {
            // Implementation functions are bound to the right transport parameters
            clean_up = [this, &transport]()
                {
                    transport.CloseOutputChannel(socket_);
                };

            send_lambda_ = [this, &transport] (
                const fastrtps::rtps::octet* data,
                uint32_t dataSize,
                fastrtps::rtps::LocatorsIterator& destination_locators_begin,
                fastrtps::rtps::LocatorsIterator& destination_locators_end,
                const std::chrono::steady_clock::time_point& max_blocking_time_point) -> bool
                    {
                        return transport.send(data, dataSize, socket_, destination_locators_begin,
                                    destination_locators_end, only_multicast_purpose_, max_blocking_time_point);
                    };
        }

        virtual ~UDPSenderResource()
        {
            if (clean_up)
            {
                clean_up();
            }
        }

        static UDPSenderResource* cast(TransportInterface& transport, SenderResource* sender_resource)
        {
            UDPSenderResource* returned_resource = nullptr;

            if (sender_resource->kind() == transport.kind())
            {
                returned_resource = dynamic_cast<UDPSenderResource*>(sender_resource);
            }

            return returned_resource;
        }

    private:

        UDPSenderResource() = delete;

        UDPSenderResource(const SenderResource&) = delete;

        UDPSenderResource& operator=(const SenderResource&) = delete;

        eProsimaUDPSocket socket_;

        bool only_multicast_purpose_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // __TRANSPORT_UDPSENDERRESOURCE_HPP__
