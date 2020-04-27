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

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>

#include <mutex>
#include <map>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

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

    /**
     * Returns the DomainParticipantFactory singleton.
     * @return The DomainParticipantFactory singleton.
     */
    RTPS_DllAPI static DomainParticipantFactory* get_instance();

    /**
     * Create a Participant.
     * @param domain_id Domain Id.
     * @param qos DomainParticipantQos Reference.
     * @param listener DomainParticipantListener Pointer.
     * @param mask StatusMask Reference.
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    RTPS_DllAPI DomainParticipant* create_participant(
            DomainId_t domain_id,
            const DomainParticipantQos& qos = PARTICIPANT_QOS_DEFAULT,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Participant.
     * @param domain_id Domain Id.
     * @param profile Participant profile name.
     * @param listener DomainParticipantListener Pointer.
     * @param mask StatusMask Reference.
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    RTPS_DllAPI DomainParticipant* create_participant_with_profile(
            DomainId_t domain_id,
            const std::string& profile_name,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * This operation retrieves a previously created DomainParticipant belonging to specified domain_id.
     * If no such DomainParticipant exists, the operation will return 'nullptr'.
     * If multiple DomainParticipant entities belonging to that domain_id exist,
     * then the operation will return one of them. It is not specified which one.
     * @param domain_id
     * @return previously created DomainParticipant
     */
    RTPS_DllAPI DomainParticipant* lookup_participant(
            DomainId_t domain_id) const;

    /**
     * Returns all participants that belongs to the specified domain_id.
     * @param domain_id
     * @return
     */
    RTPS_DllAPI std::vector<DomainParticipant*> lookup_participants(
            DomainId_t domain_id) const;

    //! Fills qos with the default values.
    RTPS_DllAPI ReturnCode_t get_default_participant_qos(
            DomainParticipantQos& qos) const;

    RTPS_DllAPI const DomainParticipantQos& get_default_participant_qos() const;

    RTPS_DllAPI ReturnCode_t set_default_participant_qos(
            const DomainParticipantQos& qos);

    /**
     * Remove a Participant and all associated publishers and subscribers.
     * @param part Pointer to the participant.
     * @return One of the standard return codes.
     */
    RTPS_DllAPI ReturnCode_t delete_participant(
            DomainParticipant* part);

    /**
     * Load profiles from default XML file.
     * @return One of the standard return codes.
     */
    RTPS_DllAPI ReturnCode_t load_profiles();

    /**
     * Load profiles from XML file.
     * @param xml_profile_file XML profile file.
     * @return One of the standard return codes.
     */
    RTPS_DllAPI ReturnCode_t load_XML_profiles_file(
            const std::string& xml_profile_file);

    /**
     * This operation returns the value of the DomainParticipantFactory QoS policies.
     * @param qos
     * @return ReturnCode
     */
    RTPS_DllAPI ReturnCode_t get_qos(
            DomainParticipantFactoryQos& qos) const;

    /**
     * This operation sets the value of the DomainParticipantFactory QoS policies. These policies
     * control the behavior of the object a factory for entities.
     *
     * Note that despite having QoS, the DomainParticipantFactory is not an Entity.
     *
     * This operation will check that the resulting policies are self consistent; if they are not,
     * the operation will have no effect and return INCONSISTENT_POLICY.
     * @param qos
     * @return ReturnCode
     */
    RTPS_DllAPI ReturnCode_t set_qos(
            const DomainParticipantFactoryQos& qos);

private:

    friend class DomainParticipantFactoryReleaser;
    friend class DomainParticipant;

    std::map<DomainId_t, std::vector<DomainParticipantImpl*> > participants_;

    DomainParticipantFactory();

    virtual ~DomainParticipantFactory();

    void reset_default_participant_qos();

    static bool delete_instance();

    static void set_qos(
            DomainParticipantFactoryQos& to,
            const DomainParticipantFactoryQos& from,
            bool first_time);

    static ReturnCode_t check_qos(
            const DomainParticipantFactoryQos& qos);

    static bool can_qos_be_updated(
            const DomainParticipantFactoryQos& to,
            const DomainParticipantFactoryQos& from);

    void participant_has_been_deleted(
            DomainParticipantImpl* part);

    mutable std::mutex mtx_participants_;

    mutable bool default_xml_profiles_loaded;

    DomainParticipantFactoryQos factory_qos_;

    DomainParticipantQos default_participant_qos_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_DOMAINPARTICIPANT_HPP_*/
