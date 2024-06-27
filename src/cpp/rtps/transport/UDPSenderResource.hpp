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

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>

#include <rtps/transport/ChainingSenderResource.hpp>
#include <rtps/transport/UDPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class UDPSenderResource : public SenderResource
{
public:

    UDPSenderResource(
            UDPTransportInterface& transport,
            eProsimaUDPSocket& socket,
            bool only_multicast_purpose = false,
            bool whitelisted = false)
        : SenderResource(transport.kind())
        , socket_(moveSocket(socket))
        , only_multicast_purpose_(only_multicast_purpose)
        , whitelisted_(whitelisted)
        , transport_(transport)
    {
        // Implementation functions are bound to the right transport parameters
        clean_up = [this, &transport]()
                {
                    transport.SenderResourceHasBeenClosed(socket_);
                };

        send_buffers_lambda_ = [this, &transport](
            const std::vector<NetworkBuffer>& buffers,
            uint32_t total_bytes,
            LocatorsIterator* destination_locators_begin,
            LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& max_blocking_time_point) -> bool
                {
                    return transport.send(buffers, total_bytes, socket_, destination_locators_begin,
                                   destination_locators_end, only_multicast_purpose_, whitelisted_,
                                   max_blocking_time_point);
                };
    }

    virtual ~UDPSenderResource()
    {
        if (clean_up)
        {
            clean_up();
        }
    }

    void add_locators_to_list(
            LocatorList& locators) const override
    {
        Locator locator;
        auto local_endpoint = getSocketPtr(socket_)->local_endpoint();
        transport_.endpoint_to_locator(local_endpoint, locator);
        locators.push_back(locator);
    }

    bool check_ip_address(
            const Locator& locator) const
    {
        Locator sender_resource_locator;
        auto local_endpoint = getSocketPtr(socket_)->local_endpoint();
        transport_.endpoint_to_locator(local_endpoint, sender_resource_locator);
        return memcmp(&sender_resource_locator.address[12], &locator.address[12], 4) == 0;
    }

    static UDPSenderResource* cast(
            TransportInterface& transport,
            SenderResource* sender_resource)
    {
        UDPSenderResource* returned_resource = nullptr;

        if (sender_resource->kind() == transport.kind())
        {
            returned_resource = dynamic_cast<UDPSenderResource*>(sender_resource);

            //! May be chained
            if (!returned_resource)
            {
                auto chaining_sender = dynamic_cast<ChainingSenderResource*>(sender_resource);

                if (chaining_sender)
                {
                    returned_resource = dynamic_cast<UDPSenderResource*>(chaining_sender->lower_sender_cast());
                }
            }
        }

        return returned_resource;
    }

private:

    UDPSenderResource() = delete;

    UDPSenderResource(
            const SenderResource&) = delete;

    UDPSenderResource& operator =(
            const SenderResource&) = delete;

    eProsimaUDPSocket socket_;

    bool only_multicast_purpose_;
    bool whitelisted_;

    UDPTransportInterface& transport_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // __TRANSPORT_UDPSENDERRESOURCE_HPP__
