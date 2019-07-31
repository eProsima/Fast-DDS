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

/**
 * @file Participant.h
 *
 */

#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>

#include <utility>

namespace eprosima {
namespace fastrtps{

class ParticipantImpl;
class ParticipantAttributes;

namespace rtps
{
    class WriterProxyData;
    class ReaderProxyData;
    class ResourceEvent;
    class RTPSParticipant;
}

/**
 * Class Participant used to group Publishers and Subscribers into a single working unit.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI Participant
{
    public:
        /**
         *	Get the rtps::GUID_t of the associated RTPSParticipant.
        * @return rtps::GUID_t
        */
        const rtps::GUID_t& getGuid() const;

        /**
         * Get the ParticipantAttributes.
         * @return ParticipantAttributes.
         */
        const ParticipantAttributes& getAttributes() const;

        /**
         * Called when using a StaticEndpointDiscovery mechanism different that the one
         * included in FastRTPS, for example when communicating with other implementations.
         * It indicates to the Participant that an Endpoint from the XML has been discovered and
         * should be activated.
         * @param partguid Participant rtps::GUID_t.
         * @param userId User defined ID as shown in the XML file.
         * @param kind EndpointKind (WRITER or READER)
         * @return True if correctly found and activated.
         */
        bool newRemoteEndpointDiscovered(
                const rtps::GUID_t& partguid,
                uint16_t userId,
                rtps::EndpointKind_t kind);

        /**
         * Returns a list with the participant names.
         * @return list of participant names.
         */
        std::vector<std::string> getParticipantNames() const;

        /**
         * @brief Asserts liveliness of manual by participant publishers
         */
        void assert_liveliness();

        rtps::ResourceEvent& get_resource_event() const;

    private:
        Participant();

        virtual ~Participant();

        ParticipantImpl* mp_impl;

        friend class Domain;

        friend class ParticipantImpl;
};

}
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */
