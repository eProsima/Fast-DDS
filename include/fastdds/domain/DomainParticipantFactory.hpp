// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DomainParticipantFactory.hpp
 *
 */

#ifndef _FASTDDS_DOMAINPARTICIPANT_HPP_
#define _FASTDDS_DOMAINPARTICIPANT_HPP_

#include "../../fastrtps/attributes/ParticipantAttributes.h"
#include <mutex>
#include <map>

namespace eprosima{
namespace fastdds{

class DomainParticipantListener;
class DomainParticipant;
class DomainParticipantImpl;

/**
 * Class DomainParticipantFactory
 *  @ingroup FASTDDS_MODULE
 */
class DomainParticipantFactory
{

public:

    RTPS_DllAPI static DomainParticipantFactory* get_instance();

    RTPS_DllAPI static bool delete_instance();

    /**
     * Create a Participant from a profile name.
     * @param participant_profile Participant profile name.
     * @param listen ParticipantListener Pointer.
     * @return Participant pointer. (nullptr if not created.)
     */
    RTPS_DllAPI DomainParticipant* create_participant(
            const std::string& participant_profile,
            DomainParticipantListener* listen = nullptr);

    /**
     * Create a Participant.
     * @param att Participant Attributes.
     * @param listen ParticipantListener Pointer.
     * @return Participant pointer. (nullptr if not created.)
     */
    RTPS_DllAPI DomainParticipant* create_participant(
            const fastrtps::ParticipantAttributes& att,
            DomainParticipantListener* listen = nullptr);

    RTPS_DllAPI DomainParticipant* lookup_participant(
            uint8_t domain_id) const;

    //!Fills participant_attributes with the default values.
    RTPS_DllAPI bool get_default_participant_qos(
            fastrtps::ParticipantAttributes& participant_qos) const;

    RTPS_DllAPI bool set_default_participant_qos(
            const fastrtps::ParticipantAttributes& participant_qos);

    /**
     * Remove a Participant and all associated publishers and subscribers.
     * @param part Pointer to the participant.
     * @return True if correctly removed.
     */
    RTPS_DllAPI bool delete_participant(
            DomainParticipant* part);

    /**
     * Load profiles from XML file.
     * @param xml_profile_file XML profile file.
     * @return True if correctly loaded.
     */
    RTPS_DllAPI bool load_XML_profiles_file(
            const std::string& xml_profile_file);

    // TODO set/get DomainParticipantFactoryQos

private:

    std::map<uint8_t, DomainParticipantImpl*> participants_;

    DomainParticipantFactory();

    virtual ~DomainParticipantFactory();

    mutable std::mutex mtx_participants_;

    mutable bool default_xml_profiles_loaded;


};

} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_DOMAINPARTICIPANT_HPP_*/
