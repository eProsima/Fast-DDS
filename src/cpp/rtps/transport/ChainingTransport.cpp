#include <fastdds/rtps/transport/ChainingTransport.h>
#include "ChainingSenderResource.hpp"
#include "ChainingReceiverResource.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

bool ChainingTransport::OpenInputChannel(
        const fastrtps::rtps::Locator_t& loc,
        TransportReceiverInterface* receiver_interface,
        uint32_t max_message_size)
{
    auto iterator = receiver_resources_.find(loc);

    if (iterator == receiver_resources_.end())
    {
        ChainingReceiverResource* receiver_resource = new ChainingReceiverResource(*this, receiver_interface);
        receiver_resources_.emplace(loc, receiver_resource);
        return low_level_transport_->OpenInputChannel(loc, receiver_resource, max_message_size);
    }

    return true;
}

bool ChainingTransport::OpenOutputChannel(
        SendResourceList& sender_resource_list,
        const fastrtps::rtps::Locator_t& loc)
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

}
}
}
