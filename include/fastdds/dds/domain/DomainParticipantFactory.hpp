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
     * @param listener DomainParticipantListener Pointer (default: nullptr)
     * @param mask StatusMask Reference (default: all)
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    RTPS_DllAPI DomainParticipant* create_participant(
            DomainId_t domain_id,
            const DomainParticipantQos& qos,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Participant.
     * @param domain_id Domain Id.
     * @param profile_name Participant profile name.
     * @param listener DomainParticipantListener Pointer (default: nullptr)
     * @param mask StatusMask Reference (default: all)
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    RTPS_DllAPI DomainParticipant* create_participant_with_profile(
            DomainId_t domain_id,
            const std::string& profile_name,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Participant.
     * @param profile_name Participant profile name.
     * @param listener DomainParticipantListener Pointer (default: nullptr)
     * @param mask StatusMask Reference (default: all)
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    RTPS_DllAPI DomainParticipant* create_participant_with_profile(
            const std::string& profile_name,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * This operation retrieves a previously created DomainParticipant belonging to specified domain_id.
     * If no such DomainParticipant exists, the operation will return 'nullptr'.
     * If multiple DomainParticipant entities belonging to that domain_id exist,
     * then the operation will return one of them. It is not specified which one.
     * @param domain_id
     * @return previously created DomainParticipant within the specified domain
     */
    RTPS_DllAPI DomainParticipant* lookup_participant(
            DomainId_t domain_id) const;

    /**
     * Returns all participants that belongs to the specified domain_id.
     * @param domain_id
     * @return previously created DomainParticipants within the specified domain
     */
    RTPS_DllAPI std::vector<DomainParticipant*> lookup_participants(
            DomainId_t domain_id) const;

    /**
     * @brief This operation retrieves the default value of the DomainParticipant QoS, that is, the QoS policies which will
     * be used for newly created DomainParticipant entities in the case where the QoS policies are defaulted in the
     * create_participant operation.
     * The values retrieved get_default_participant_qos will match the set of values specified on the last successful call
     * to set_default_participant_qos, or else, if the call was never made, the default values.
     * @param qos DomainParticipantQos where the qos is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_default_participant_qos(
            DomainParticipantQos& qos) const;

    /**
     * @brief This operation retrieves the default value of the DomainParticipant QoS, that is, the QoS policies which will
     * be used for newly created DomainParticipant entities in the case where the QoS policies are defaulted in the
     * create_participant operation.
     * The values retrieved get_default_participant_qos will match the set of values specified on the last successful call
     * to set_default_participant_qos, or else, if the call was never made, the default values.
     * @return A reference to the default DomainParticipantQos
     */
    RTPS_DllAPI const DomainParticipantQos& get_default_participant_qos() const;

    /**
     * @brief This operation sets a default value of the DomainParticipant QoS policies which will be used for
     * newly created DomainParticipant entities in the case where the QoS policies are defaulted in the
     * create_participant operation.
     *
     * This operation will check that the resulting policies are self consistent; if they are not, the operation
     * will have no effect and return INCONSISTENT_POLICY.
     *
     * The special value PARTICIPANT_QOS_DEFAULT may be passed to this operation to indicate that the default
     * QoS should be reset back to the initial values the factory would use, that is the values that would be
     * used if the set_default_participant_qos operation had never been called.
     *
     * @param qos DomainParticipantQos to be set
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_default_participant_qos(
            const DomainParticipantQos& qos);

    /**
     * Fills the DomainParticipantQos with the values of the XML profile.
     * @param profile_name DomainParticipant profile name.
     * @param qos DomainParticipantQos object where the qos is returned.
     * @return RETCODE_OK if the profile exists. RETCODE_BAD_PARAMETER otherwise.
     */
    RTPS_DllAPI ReturnCode_t get_participant_qos_from_profile(
            const std::string& profile_name,
            DomainParticipantQos& qos) const;

    /**
     * Remove a Participant and all associated publishers and subscribers.
     * @param part Pointer to the participant.
     * @return RETCODE_PRECONDITION_NOT_MET if the participant has active entities, RETCODE_OK if the participant is correctly
     * deleted and RETCODE_ERROR otherwise.
     */
    RTPS_DllAPI ReturnCode_t delete_participant(
            DomainParticipant* part);

    /**
     * Load profiles from default XML file.
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t load_profiles();

    /**
     * Load profiles from XML file.
     * @param xml_profile_file XML profile file.
     * @return RETCODE_OK if it is correctly loaded, RETCODE_ERROR otherwise.
     */
    RTPS_DllAPI ReturnCode_t load_XML_profiles_file(
            const std::string& xml_profile_file);

    /**
     * This operation returns the value of the DomainParticipantFactory QoS policies.
     * @param qos DomaParticipantFactoryQos reference where the qos is returned
     * @return RETCODE_OK
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
     * @param qos DomainParticipantFactoryQos to be set.
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_qos(
            const DomainParticipantFactoryQos& qos);

protected:

    friend class DomainParticipant;

    std::map<DomainId_t, std::vector<DomainParticipantImpl*>> participants_;

    DomainParticipantFactory();

    virtual ~DomainParticipantFactory();

    DomainParticipantFactory (
            const DomainParticipantFactory&) = delete;

    void operator = (
            const DomainParticipantFactory&) = delete;

    void reset_default_participant_qos();

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
