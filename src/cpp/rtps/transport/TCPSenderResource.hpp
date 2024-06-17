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

#ifndef __TRANSPORT_TCPSENDERRESOURCE_HPP__
#define __TRANSPORT_TCPSENDERRESOURCE_HPP__

#include <fastdds/rtps/common/LocatorsIterator.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>

#include <rtps/transport/ChainingSenderResource.hpp>
#include <rtps/transport/TCPChannelResource.h>
#include <rtps/transport/TCPTransportInterface.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class TCPSenderResource : public SenderResource
{
public:

    TCPSenderResource(
            TCPTransportInterface& transport,
            Locator_t& locator)
        : SenderResource(transport.kind())
        , locator_(locator)
    {
        // Implementation functions are bound to the right transport parameters
        clean_up = [this, &transport]()
                {
                    transport.SenderResourceHasBeenClosed(locator_);
                };

        send_buffers_lambda_ = [this, &transport](
            const std::vector<NetworkBuffer>& buffers,
            uint32_t total_bytes,
            LocatorsIterator* destination_locators_begin,
            LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point&) -> bool
                {
                    return transport.send(buffers, total_bytes, locator_, destination_locators_begin,
                                   destination_locators_end);
                };
    }

    virtual ~TCPSenderResource()
    {
        if (clean_up)
        {
            clean_up();
        }
    }

    Locator_t& locator()
    {
        return locator_;
    }

    static TCPSenderResource* cast(
            const TransportInterface& transport,
            SenderResource* sender_resource)
    {
        TCPSenderResource* returned_resource = nullptr;

        if (sender_resource->kind() == transport.kind())
        {
            returned_resource = dynamic_cast<TCPSenderResource*>(sender_resource);

            //! May be chained
            if (!returned_resource)
            {
                auto chaining_sender = dynamic_cast<ChainingSenderResource*>(sender_resource);

                if (chaining_sender)
                {
                    returned_resource = dynamic_cast<TCPSenderResource*>(chaining_sender->lower_sender_cast());
                }
            }
        }

        return returned_resource;
    }

private:

    TCPSenderResource() = delete;

    TCPSenderResource(
            const SenderResource&) = delete;

    TCPSenderResource& operator =(
            const SenderResource&) = delete;

    Locator_t locator_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // __TRANSPORT_UDPSENDERRESOURCE_HPP__
