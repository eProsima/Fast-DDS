// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * InfoReplyMsg.hpp
 *
 */

namespace eprosima {
namespace fastrtps {
namespace rtps {

static bool addSubmessageInfoReplyV4(
        CDRMessage_t* msg,
        const Locator_t& unicast_locator,
        const Locator_t& multicast_locator)
{
    return false;
}

bool RTPSMessageCreator::addSubmessageInfoReply(
        CDRMessage_t* msg,
        const LocatorList_t& unicast_locators,
        const LocatorList_t& multicast_locators)
{
    bool compact_unicast = (unicast_locators.size() == 1) && (LOCATOR_KIND_UDPv4 == unicast_locators.begin()->kind);
    bool compact_version = compact_unicast && (multicast_locators.size() <= 1);
    if ( (multicast_locators.size() == 1) && (LOCATOR_KIND_UDPv4 != multicast_locators.begin()->kind))
    {
        compact_version = false;
    }

    if (compact_version)
    {
        Locator_t multicast_locator;
        if (multicast_locators.size() == 1)
        {
            multicast_locator = *multicast_locators.begin();
        }
        return addSubmessageInfoReplyV4(msg, *unicast_locators.begin(), multicast_locator);
    }

    return false;
}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
