// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _RTPS_TRANSPORT_CHAININGSENDERRESOURCE_HPP_
#define _RTPS_TRANSPORT_CHAININGSENDERRESOURCE_HPP_

#include <fastdds/rtps/network/SenderResource.h>
#include <fastdds/rtps/transport/ChainingTransport.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ChainingSenderResource : public fastrtps::rtps::SenderResource
{
public:

    ChainingSenderResource(
            ChainingTransport& transport,
            std::unique_ptr<SenderResource>& low_sender_resource)
        : SenderResource(transport.kind())
        , low_sender_resource_(std::move(low_sender_resource))
    {
        // Implementation functions are bound to the right transport parameters
        clean_up = []()
                {
                    // No cleanup is required.
                    // low_sender_resources_ makes its clean up on destruction.
                };

        send_lambda_ = [this, &transport](
            const fastrtps::rtps::octet* data,
            uint32_t dataSize,
            fastrtps::rtps::LocatorsIterator* destination_locators_begin,
            fastrtps::rtps::LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& timeout) -> bool
                {
                    if (low_sender_resource_)
                    {
                        return transport.send(low_sender_resource_.get(), data, dataSize,
                                       destination_locators_begin, destination_locators_end, timeout);
                    }

                    return false;
                };
    }

    virtual ~ChainingSenderResource()
    {
        if (clean_up)
        {
            clean_up();
        }
    }

private:

    ChainingSenderResource() = delete;

    ChainingSenderResource(
            const SenderResource&) = delete;

    ChainingSenderResource& operator =(
            const SenderResource&) = delete;

    std::unique_ptr<SenderResource> low_sender_resource_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_TRANSPORT_CHAININGSENDERRESOURCE_HPP_
