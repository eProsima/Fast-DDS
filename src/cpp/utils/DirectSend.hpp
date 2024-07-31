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


#ifndef UTILS__DIRECTSEND_HPP
#define UTILS__DIRECTSEND_HPP

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/Endpoint.h>

#include <rtps/builtin/discovery/participant/DirectMessageSender.hpp>
#include <rtps/builtin/discovery/participant/FakeWriter.hpp>
#include <fastdds/rtps/messages/RTPSMessageGroup.h>


namespace eprosima {
namespace fastdds {
namespace rtps {

inline void direct_send(
        fastrtps::rtps::RTPSParticipantImpl* participant,
        LocatorList& locators,
        std::vector<fastrtps::rtps::GUID_t>& remote_readers,
        fastrtps::rtps::CacheChange_t& change,
        fastrtps::rtps::Endpoint& sender_endpt)
{
    fastrtps::rtps::DirectMessageSender sender(participant, &remote_readers, &locators);
    fastrtps::rtps::RTPSMessageGroup group(participant, &sender_endpt, &sender);
    if (!group.add_data(change, false))
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "Error sending announcement from client to servers");
    }
}

inline void direct_send(
        fastrtps::rtps::RTPSParticipantImpl* participant,
        LocatorList& locators,
        fastrtps::rtps::CacheChange_t& change)
{
    FakeWriter writer(participant, fastrtps::rtps::c_EntityId_SPDPWriter);
    std::vector<fastrtps::rtps::GUID_t> remote_readers;
    direct_send(participant, locators, remote_readers, change, writer);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // UTILS__DIRECTSEND_HPP
