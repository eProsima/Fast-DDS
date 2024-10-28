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

/**
 * @file FakeWriter.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__FAKEWRITER_HPP_
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__FAKEWRITER_HPP_

#include <fastdds/rtps/Endpoint.hpp>

#include <rtps/participant/RTPSParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * An RTPS writer simulator used to send messages with a \ref DirectMessageSender.
 */
class FakeWriter : public Endpoint
{
public:

    FakeWriter(
            RTPSParticipantImpl* participant,
            const EntityId_t& entity_id)
        : Endpoint(participant, { participant->getGuid().guidPrefix, entity_id }, {})
    {
        m_att.endpointKind = EndpointKind_t::WRITER;
#if HAVE_SECURITY
        participant->set_endpoint_rtps_protection_supports(this, false);
#endif // HAVE_SECURITY
    }

    virtual ~FakeWriter() override = default;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif /* FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__FAKEWRITER_HPP_ */
