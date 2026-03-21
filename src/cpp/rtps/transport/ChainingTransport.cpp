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

#include <fastdds/rtps/transport/ChainingTransport.hpp>
#include "ChainingSenderResource.hpp"
#include "ChainingReceiverResource.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

void ChainingReceiverResourceDeleter::operator ()(
        ChainingReceiverResource* p)
{
    delete p;
}

bool ChainingTransport::OpenInputChannel(
        const Locator_t& loc,
        TransportReceiverInterface* receiver_interface,
        uint32_t max_message_size)
{
    auto iterator = receiver_resources_.find(loc);

    if (iterator == receiver_resources_.end())
    {
        ChainingReceiverResource* receiver_resource = new ChainingReceiverResource(*this, receiver_interface);
        receiver_resources_.emplace(loc, ChainingReceiverResourceReferenceType(receiver_resource));
        return low_level_transport_->OpenInputChannel(loc, receiver_resource, max_message_size);
    }

    return true;
}

bool ChainingTransport::OpenOutputChannel(
        SendResourceList& sender_resource_list,
        const Locator_t& loc)
{
    size_t original_size = sender_resource_list.size();
    bool returned_value = low_level_transport_->OpenOutputChannel(sender_resource_list, loc);

    if (returned_value)
    {
        for (size_t current_position = original_size; current_position < sender_resource_list.size();
                ++current_position)
        {
            ChainingSenderResource* sender_resource = new ChainingSenderResource(*this,
                            sender_resource_list.at(current_position));
            sender_resource_list.at(current_position).reset(sender_resource);
        }
    }

    return returned_value;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
