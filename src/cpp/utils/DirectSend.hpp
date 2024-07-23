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

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/Endpoint.hpp>

#include <rtps/builtin/discovery/participant/DirectMessageSender.hpp>
#include <rtps/builtin/discovery/participant/FakeWriter.hpp>
#include <rtps/messages/RTPSMessageGroup.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {

class Endpoint;
class RTPSParticipantImpl;
struct CacheChange_t;


inline void direct_send(
        RTPSParticipantImpl* participant,
        LocatorList& locators,
        std::vector<GUID_t>& remote_readers,
        CacheChange_t& change,
        fastdds::rtps::Endpoint& sender_endpt)
{
    DirectMessageSender sender(participant, &remote_readers, &locators);
    RTPSMessageGroup group(participant, &sender_endpt, &sender);
    if (!group.add_data(change, false))
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "Error sending announcement from client to servers");
    }
}

inline void direct_send(
        RTPSParticipantImpl* participant,
        LocatorList& locators,
        CacheChange_t& change)
{
    FakeWriter writer(participant, c_EntityId_SPDPWriter);
    std::vector<GUID_t> remote_readers;
    direct_send(participant, locators, remote_readers, change, writer);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // UTILS__DIRECTSEND_HPP
