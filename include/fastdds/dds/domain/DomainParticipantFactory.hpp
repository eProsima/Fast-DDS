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

#ifndef FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANTFACTORY_HPP
#define FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANTFACTORY_HPP

#include <map>
#include <memory>
#include <mutex>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantExtendedQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/LibrarySettings.hpp>

namespace eprosima {

namespace fastdds {
namespace rtps {

class RTPSDomainImpl;

namespace detail {
class TopicPayloadPoolRegistry;
}  // namespace detail
}  // namespace rtps

namespace dds {

class DomainParticipantListener;
class DomainParticipant;
class DomainParticipantImpl;

namespace detail {
struct LogResources;
}  // namespace detail

/**
 * Class DomainParticipantFactory
 *
 *  @ingroup FASTDDS_MODULE
 */
class DomainParticipantFactory
{

public:

    /**
     * Returns the DomainParticipantFactory singleton instance.
     *
     * @return A raw pointer to the DomainParticipantFactory singleton instance.
     */
    FASTDDS_EXPORTED_API static DomainParticipantFactory* get_instance();

    /**
     * Returns the DomainParticipantFactory singleton instance.
     *
     * @return A shared pointer to the DomainParticipantFactory singleton instance.
     */
    FASTDDS_EXPORTED_API static std::shared_ptr<DomainParticipantFactory> get_shared_instance();

    /**
     * Create a Participant.
     *
     * @param domain_id Domain Id.
     * @param qos DomainParticipantQos Reference.
     * @param listener DomainParticipantListener Pointer (default: nullptr)
     * @param mask StatusMask Reference (default: all)
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    FASTDDS_EXPORTED_API DomainParticipant* create_participant(
            DomainId_t domain_id,
            const DomainParticipantQos& qos,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Participant.
     *
     * @param extended_qos DomainParticipantExtendedQos Reference.
     * @param listener DomainParticipantListener Pointer (default: nullptr)
     * @param mask StatusMask Reference (default: all)
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    FASTDDS_EXPORTED_API DomainParticipant* create_participant(
            const DomainParticipantExtendedQos& extended_qos,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Participant with default domain id and qos.
     *
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    FASTDDS_EXPORTED_API DomainParticipant* create_participant_with_default_profile();


    /**
     * Create a Participant with default domain id and qos.
     *
     * @return DomainParticipant pointer. (nullptr if not created.)
     * @param listener DomainParticipantListener Pointer
     * @param mask StatusMask Reference
     */
    FASTDDS_EXPORTED_API DomainParticipant* create_participant_with_default_profile(
            DomainParticipantListener* listener,
            const StatusMask& mask);

    /**
     * Create a Participant.
     *
     * @param domain_id Domain Id.
     * @param profile_name Participant profile name.
     * @param listener DomainParticipantListener Pointer (default: nullptr)
     * @param mask StatusMask Reference (default: all)
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    FASTDDS_EXPORTED_API DomainParticipant* create_participant_with_profile(
            DomainId_t domain_id,
            const std::string& profile_name,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Participant.
     *
     * @param profile_name Participant profile name.
     * @param listener DomainParticipantListener Pointer (default: nullptr)
     * @param mask StatusMask Reference (default: all)
     * @return DomainParticipant pointer. (nullptr if not created.)
     */
    FASTDDS_EXPORTED_API DomainParticipant* create_participant_with_profile(
            const std::string& profile_name,
            DomainParticipantListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * This operation retrieves a previously created DomainParticipant belonging to specified domain_id.
     * If no such DomainParticipant exists, the operation will return 'nullptr'.
     * If multiple DomainParticipant entities belonging to that domain_id exist,
     * then the operation will return one of them. It is not specified which one.
     *
     * @param domain_id
     * @return previously created DomainParticipant within the specified domain
     */
    FASTDDS_EXPORTED_API DomainParticipant* lookup_participant(
            DomainId_t domain_id) const;

    /**
     * Returns all participants that belongs to the specified domain_id.
     *
     * @param domain_id
     * @return previously created DomainParticipants within the specified domain
     */
    FASTDDS_EXPORTED_API std::vector<DomainParticipant*> lookup_participants(
            DomainId_t domain_id) const;

    /**
     * @brief This operation retrieves the default value of the DomainParticipant QoS, that is, the QoS policies which will
     * be used for newly created DomainParticipant entities in the case where the QoS policies are defaulted in the
     * create_participant operation.
     * The values retrieved get_default_participant_qos will match the set of values specified on the last successful call
     * to set_default_participant_qos, or else, if the call was never made, the default values.
     *
     * @param qos DomainParticipantQos where the qos is returned
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_participant_qos(
            DomainParticipantQos& qos) const;

    /**
     * @brief This operation retrieves the default value of the DomainParticipant QoS, that is, the QoS policies which will
     * be used for newly created DomainParticipant entities in the case where the QoS policies are defaulted in the
     * create_participant operation.
     * The values retrieved get_default_participant_qos will match the set of values specified on the last successful call
     * to set_default_participant_qos, or else, if the call was never made, the default values.
     *
     * @return A reference to the default DomainParticipantQos
     */
    FASTDDS_EXPORTED_API const DomainParticipantQos& get_default_participant_qos() const;

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
    FASTDDS_EXPORTED_API ReturnCode_t set_default_participant_qos(
            const DomainParticipantQos& qos);

    /**
     * Fills the @ref DomainParticipantQos with the values of the XML profile.
     *
     * @param profile_name DomainParticipant profile name.
     * @param qos @ref DomainParticipantQos object where the qos is returned.
     * @return RETCODE_OK if the profile exists. RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_participant_qos_from_profile(
            const std::string& profile_name,
            DomainParticipantQos& qos) const;

    /**
     * Fills the @ref DomainParticipantQos with the first DomainParticipant profile found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DomainParticipantQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_participant_qos_from_xml(
            const std::string& xml,
            DomainParticipantQos& qos) const;

    /**
     * Fills the @ref DomainParticipantQos with the DomainParticipant profile with \c profile_name to be found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DomainParticipantQos object where the qos is returned.
     * @param profile_name DomainParticipant profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_participant_qos_from_xml(
            const std::string& xml,
            DomainParticipantQos& qos,
            const std::string& profile_name) const;

    /**
     * Fills the @ref DomainParticipantQos with the default DomainParticipant profile found in the provided XML (if there is).
     *
     * @note This method does not update the default participant qos (returned by \c get_default_participant_qos).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref DomainParticipantQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_participant_qos_from_xml(
            const std::string& xml,
            DomainParticipantQos& qos) const;

    /**
     * Fills the @ref DomainParticipantExtendedQos with the values of the XML profile.
     *
     * @param profile_name DomainParticipant profile name.
     * @param extended_qos DomainParticipantExtendedQos object where the domain and qos are returned.
     * @return RETCODE_OK if the profile exists. RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_participant_extended_qos_from_profile(
            const std::string& profile_name,
            DomainParticipantExtendedQos& extended_qos) const;

    /**
     * Fills the @ref DomainParticipantExtendedQos with the first DomainParticipant profile found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c extended_qos structure.
     * @param extended_qos @ref DomainParticipantExtendedQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_participant_extended_qos_from_xml(
            const std::string& xml,
            DomainParticipantExtendedQos& extended_qos) const;

    /**
     * Fills the @ref DomainParticipantExtendedQos with the DomainParticipant profile with \c profile_name to be found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c extended_qos structure.
     * @param extended_qos @ref DomainParticipantExtendedQos object where the qos is returned.
     * @param profile_name DomainParticipant profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_participant_extended_qos_from_xml(
            const std::string& xml,
            DomainParticipantExtendedQos& extended_qos,
            const std::string& profile_name) const;

    /**
     * Fills the @ref DomainParticipantExtendedQos with the default DomainParticipant profile found in the provided XML (if there is).
     *
     * @note This method does not update the default participant extended qos.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c extended_qos structure.
     * @param extended_qos @ref DomainParticipantExtendedQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_participant_extended_qos_from_xml(
            const std::string& xml,
            DomainParticipantExtendedQos& extended_qos) const;

    /**
     * Fills the @ref DomainParticipantExtendedQos with the values of the default XML profile.
     *
     * @param extended_qos @ref DomainParticipantExtendedQos object where the domain and qos are returned.
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_participant_extended_qos_from_default_profile(
            DomainParticipantExtendedQos& extended_qos) const;

    /**
     * Remove a Participant and all associated publishers and subscribers.
     *
     * @param part Pointer to the participant.
     * @return RETCODE_PRECONDITION_NOT_MET if the participant has active entities, RETCODE_OK if the participant is correctly
     * deleted and RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_participant(
            DomainParticipant* part);

    /**
     * Load profiles from default XML file.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t load_profiles();

    /**
     * Load profiles from XML file.
     *
     * @param xml_profile_file XML profile file.
     * @return RETCODE_OK if it is correctly loaded, RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t load_XML_profiles_file(
            const std::string& xml_profile_file);

    /**
     * Load profiles from XML string.
     *
     * @param data buffer containing xml data.
     * @param length length of data
     * @return RETCODE_OK if it is correctly loaded, RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t load_XML_profiles_string(
            const char* data,
            size_t length);

    /**
     * Check the validity of the provided static discovery XML file
     *
     * @param xml_file xml file path
     * @return RETCODE_OK if the validation is successful, RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t check_xml_static_discovery(
            std::string& xml_file);

    /**
     * This operation returns the value of the DomainParticipantFactory QoS policies.
     *
     * @param qos DomaParticipantFactoryQos reference where the qos is returned
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_qos(
            DomainParticipantFactoryQos& qos) const;

    /**
     * This operation sets the value of the DomainParticipantFactory QoS policies. These policies
     * control the behavior of the object a factory for entities.
     *
     * Note that despite having QoS, the DomainParticipantFactory is not an Entity.
     *
     * This operation will check that the resulting policies are self consistent; if they are not,
     * the operation will have no effect and return INCONSISTENT_POLICY.
     *
     * @param qos DomainParticipantFactoryQos to be set.
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_qos(
            const DomainParticipantFactoryQos& qos);

    /**
     * @brief This operation returns the value of the DomainParticipant library settings.
     *
     * @param library_settings LibrarySettings reference where the settings are returned.
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_library_settings(
            LibrarySettings& library_settings) const;

    /**
     * @brief This operation sets the library settings.
     *
     * Library settings must be set before enabling the DomainParticipants.
     * Otherwise, failure of the setting operation is expected.
     *
     * @param library_settings LibrarySettings to be set.
     * @return RETCODE_PRECONDITION_NOT_MET if any DomainParticipant is already enabled.
     *         RETCODE_OK otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_library_settings(
            const LibrarySettings& library_settings);

    /**
     * @brief Get the @ref DynamicType defined in XML file.
     *        The XML file shall be previously loaded.
     *
     * @param type_name Dynamic type name.
     * @param type Reference where the Dynamic type builder is returned.
     * @return RETCODE_BAD_PARAMETER if type_name is empty.
     *         RETCODE_NO_DATA if type_name is unknown.
     *         RETCODE_OK otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_dynamic_type_builder_from_xml_by_name(
            const std::string& type_name,
            DynamicTypeBuilder::_ref_type& type);

    /**
     * @brief Return the TypeObjectRegistry member to access the public API.
     *
     * @return const xtypes::TypeObjectRegistry reference.
     */
    FASTDDS_EXPORTED_API xtypes::ITypeObjectRegistry& type_object_registry();

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

    DomainId_t default_domain_id_;

    DomainParticipantFactoryQos factory_qos_;

    DomainParticipantQos default_participant_qos_;

    std::shared_ptr<fastdds::rtps::detail::TopicPayloadPoolRegistry> topic_pool_;

    std::shared_ptr<fastdds::rtps::RTPSDomainImpl> rtps_domain_;

    std::shared_ptr<detail::LogResources> log_resources_;

    /**
     * This mutex guards the access to load the profiles.
     * Is used to lock every thread that is trying to load the profiles, so only the first one loads it and
     * until it is not finished the rest of them does not leave function \c load_profiles .
     */
    mutable std::mutex default_xml_profiles_loaded_mtx_;
};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANTFACTORY_HPP
