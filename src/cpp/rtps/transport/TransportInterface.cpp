// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/transport/TransportInterface.hpp>

#include <fastdds/rtps/common/LocatorSelectorEntry.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool TransportInterface::OpenOutputChannels(
        SendResourceList& send_resource_list,
        const LocatorSelectorEntry& locator_selector_entry)
{
    bool success = false;
    for (size_t i = 0; i < locator_selector_entry.state.multicast.size(); ++i)
    {
        size_t index = locator_selector_entry.state.multicast[i];
        success |= OpenOutputChannel(send_resource_list, locator_selector_entry.multicast[index]);
    }
    for (size_t i = 0; i < locator_selector_entry.state.unicast.size(); ++i)
    {
        size_t index = locator_selector_entry.state.unicast[i];
        success |= OpenOutputChannel(send_resource_list, locator_selector_entry.unicast[index]);
    }
    return success;
}

void TransportInterface::CloseOutputChannels(
        SendResourceList& sender_resource_list,
        const LocatorSelectorEntry& locator_selector_entry)
{
    static_cast<void>(sender_resource_list);
    static_cast<void>(locator_selector_entry);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
