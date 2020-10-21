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
 * @file DomainParticipant.hpp
 *
 */

#ifndef _FASTDDS_DOMAIN_PARTICIPANT_HPP_
#define _FASTDDS_DOMAIN_PARTICIPANT_HPP_

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastrtps/types/TypeIdentifier.h>

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/SampleIdentity.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <utility>

using eprosima::fastrtps::types::ReturnCode_t;

namespace dds {
namespace domain {
class DomainParticipant;
} // namespace domain
} // namespace dds

namespace eprosima {
namespace fastrtps {
namespace rtps {
class ResourceEvent;
} // namespace rtps

namespace types {
class TypeInformation;
} // namespace types

class ParticipantAttributes;
class PublisherAttributes;
class SubscriberAttributes;

} //namespace fastrtps

namespace fastdds {
namespace dds {

class DomainParticipantImpl;
class DomainParticipantListener;
class Publisher;
class PublisherQos;
class PublisherListener;
class Subscriber;
class SubscriberQos;
class SubscriberListener;
class TopicQos;

/**
 * Class DomainParticipant used to group Publishers and Subscribers into a single working unit.
 * @ingroup FASTDDS_MODULE
 */
class DomainParticipant : public Entity
{
public:

    /**
     * This operation returns the value of the DomainParticipant QoS policies
     * @param qos DomainParticipantQos reference where the qos is going to be returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_qos(
            DomainParticipantQos& qos) const;

    /**
     * @brief This operation returns the value of the DomainParticipant QoS policies
     * @return A reference to the DomainParticipantQos
     */
    RTPS_DllAPI const DomainParticipantQos& get_qos() const;

    /**
     * @brief This operation enables the DomainParticipant
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t enable() override;

    /**
     * This operation sets the value of the DomainParticipant QoS policies.
     * @param qos DomainParticipantQos to be set
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_qos(
            const DomainParticipantQos& qos) const;

    /**
     * Modifies the DomainParticipantListener, sets the mask to StatusMask::all()
     * @param listener new value for the DomainParticipantListener
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            DomainParticipantListener* listener);

    /**
     * Modifies the DomainParticipantListener.
     * @param listener new value for the DomainParticipantListener
     * @param mask StatusMask that holds statuses the listener responds to
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t set_listener(
            DomainParticipantListener* listener,
            const StatusMask& mask);

    /**
     * Allows accessing the DomainParticipantListener.
     * @return DomainParticipantListener pointer
     */
    RTPS_DllAPI const DomainParticipantListener* get_listener() const;

    /**
     * Create a Publisher in this Participant.
     * @param qos QoS of the Publisher.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Publisher.
     */
    RTPS_DllAPI Publisher* create_publisher(
            const PublisherQos& qos,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Publisher in this Participant.
     * @param profile_name Publisher profile name.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Publisher.
     */
    RTPS_DllAPI Publisher* create_publisher_with_profile(
            const std::string& profile_name,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Deletes an existing Publisher.
     * @param publisher to be deleted.
     * @return RETCODE_PRECONDITION_NOT_MET if the publisher does not belong to this participant or if it has active DataWriters,
     * RETCODE_OK if it is correctly deleted and RETCODE_ERROR otherwise.
     */
    RTPS_DllAPI ReturnCode_t delete_publisher(
            Publisher* publisher);

    /**
     * Create a Subscriber in this Participant.
     * @param qos QoS of the Subscriber.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Subscriber.
     */
    RTPS_DllAPI Subscriber* create_subscriber(
            const SubscriberQos& qos,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Subscriber in this Participant.
     * @param profile_name Subscriber profile name.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Subscriber.
     */
    RTPS_DllAPI Subscriber* create_subscriber_with_profile(
            const std::string& profile_name,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Deletes an existing Subscriber.
     * @param subscriber to be deleted.
     * @return RETCODE_PRECONDITION_NOT_MET if the subscriber does not belong to this participant or if it has active DataReaders,
     * RETCODE_OK if it is correctly deleted and RETCODE_ERROR otherwise.
     */
    RTPS_DllAPI ReturnCode_t delete_subscriber(
            Subscriber* subscriber);

    /**
     * Register a type in this participant.
     * @param type TypeSupport.
     * @param type_name The name that will be used to identify the Type.
     * @return RETCODE_BAD_PARAMETER if the size of the name is 0, RERCODE_PRECONDITION_NOT_MET if there is another TypeSupport
     * with the same name and RETCODE_OK if it is correctly registered.
     */
    RTPS_DllAPI ReturnCode_t register_type(
            TypeSupport type,
            const std::string& type_name);

    /**
     * Register a type in this participant.
     * @param type TypeSupport.
     * @return RETCODE_BAD_PARAMETER if the size of the name is 0, RERCODE_PRECONDITION_NOT_MET if there is another TypeSupport
     * with the same name and RETCODE_OK if it is correctly registered.
     */
    RTPS_DllAPI ReturnCode_t register_type(
            TypeSupport type);

    /**
     * Unregister a type in this participant.
     * @param typeName Name of the type
     * @return RETCODE_BAD_PARAMETER if the size of the name is 0, RERCODE_PRECONDITION_NOT_MET if there are entities using that
     * TypeSupport and RETCODE_OK if it is correctly unregistered.
     */
    RTPS_DllAPI ReturnCode_t unregister_type(
            const std::string& typeName);

    /**
     * Create a Topic in this Participant.
     * @param topic_name Name of the Topic.
     * @param type_name Data type of the Topic.
     * @param qos QoS of the Topic.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Topic.
     */
    RTPS_DllAPI Topic* create_topic(
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Topic in this Participant.
     * @param topic_name Name of the Topic.
     * @param type_name Data type of the Topic.
     * @param profile_name Topic profile name.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Topic.
     */
    RTPS_DllAPI Topic* create_topic_with_profile(
            const std::string& topic_name,
            const std::string& type_name,
            const std::string& profile_name,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Deletes an existing Topic.
     * @param topic to be deleted.
     * @return RETCODE_BAD_PARAMETER if the topic passed is a nullptr, RETCODE_PRECONDITION_NOT_MET if the topic does not belong to
     * this participant or if it is referenced by any entity and ETCODE_OK if the Topic was deleted.
     */
    RTPS_DllAPI ReturnCode_t delete_topic(
            Topic* topic);

    /**
     * Looks up an existing, locally created @ref TopicDescription, based on its name.
     * May be called on a disabled participant.
     *
     * @param topic_name Name of the @ref TopicDescription to search for.
     *
     * @return Pointer to the topic description, if it has been created locally. Otherwise, nullptr is returned.
     *
     * @remark UNSAFE. It is unsafe to lookup a topic description while another thread is creating a topic.
     */
    RTPS_DllAPI TopicDescription* lookup_topicdescription(
            const std::string& topic_name) const;

    /* TODO
       Subscriber* get_builtin_subscriber();
     */

    /* TODO
       bool ignore_participant(
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    /* TODO
       bool ignore_topic(
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    /* TODO
       bool ignore_publication(
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    /* TODO
       bool ignore_subscription(
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    /**
     * This operation retrieves the domain_id used to create the DomainParticipant.
     * The domain_id identifies the DDS domain to which the DomainParticipant belongs.
     * @return The Participant's domain_id
     */
    RTPS_DllAPI DomainId_t get_domain_id() const;

    /* TODO
       bool delete_contained_entities();
     */

    /**
     * This operation manually asserts the liveliness of the DomainParticipant.
     * This is used in combination with the LIVELINESS QoS policy to indicate to the Service that the entity
     * remains active.
     *
     * This operation needs to only be used if the DomainParticipant contains DataWriter entities with
     * the LIVELINESS set to MANUAL_BY_PARTICIPANT and it only affects the liveliness of those DataWriter entities.
     * Otherwise, it has no effect.
     *
     * @note Writing data via the write operation on a DataWriter asserts liveliness on the DataWriter itself and its
     * DomainParticipant. Consequently the use of assert_liveliness is only needed if the application is not
     * writing data regularly.
     * @return RETCODE_OK if the liveliness was asserted, RETCODE_ERROR otherwise.
     */
    RTPS_DllAPI ReturnCode_t assert_liveliness();

    /**
     * This operation sets a default value of the Publisher QoS policies which will be used for newly created
     * Publisher entities in the case where the QoS policies are defaulted in the create_publisher operation.
     *
     * This operation will check that the resulting policies are self consistent; if they are not,
     * the operation will have no effect and return false.
     *
     * The special value PUBLISHER_QOS_DEFAULT may be passed to this operation to indicate that the default QoS
     * should be reset back to the initial values the factory would use, that is the values that would be used
     * if the set_default_publisher_qos operation had never been called.
     * @param qos PublisherQos to be set
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_default_publisher_qos(
            const PublisherQos& qos);

    /**
     * This operation retrieves the default value of the Publisher QoS, that is, the QoS policies which will be used
     * for newly created Publisher entities in the case where the QoS policies are defaulted in the
     * create_publisher operation.
     *
     * The values retrieved get_default_publisher_qos will match the set of values specified on the last successful
     * call to set_default_publisher_qos, or else, if the call was never made, the default values.
     * @return Current default publisher qos.
     */
    RTPS_DllAPI const PublisherQos& get_default_publisher_qos() const;

    /**
     * This operation retrieves the default value of the Publisher QoS, that is, the QoS policies which will be used
     * for newly created Publisher entities in the case where the QoS policies are defaulted in the
     * create_publisher operation.
     *
     * The values retrieved get_default_publisher_qos will match the set of values specified on the last successful
     * call to set_default_publisher_qos, or else, if the call was never made, the default values.
     * @param qos PublisherQos reference where the default_publisher_qos is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_default_publisher_qos(
            PublisherQos& qos) const;

    /**
     * Fills the PublisherQos with the values of the XML profile.
     * @param profile_name Publisher profile name.
     * @param qos PublisherQos object where the qos is returned.
     * @return RETCODE_OK if the profile exists. RETCODE_BAD_PARAMETER otherwise.
     */
    RTPS_DllAPI ReturnCode_t get_publisher_qos_from_profile(
            const std::string& profile_name,
            PublisherQos& qos) const;

    /**
     * This operation sets a default value of the Subscriber QoS policies that will be used for newly created
     * Subscriber entities in the case where the QoS policies are defaulted in the create_subscriber operation.
     *
     * This operation will check that the resulting policies are self consistent; if they are not,
     * the operation will have no effect and return false.
     *
     * The special value SUBSCRIBER_QOS_DEFAULT may be passed to this operation to indicate that the default QoS
     * should be reset back to the initial values the factory would use, that is the values that would be used
     * if the set_default_subscriber_qos operation had never been called.
     * @param qos SubscriberQos to be set
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_default_subscriber_qos(
            const SubscriberQos& qos);

    /**
     * This operation retrieves the default value of the Subscriber QoS, that is, the QoS policies which will be used
     * for newly created Subscriber entities in the case where the QoS policies are defaulted in the
     * create_subscriber operation.
     *
     * The values retrieved get_default_subscriber_qos will match the set of values specified on the last successful
     * call to set_default_subscriber_qos, or else, if the call was never made, the default values.
     * @return Current default subscriber qos.
     */
    RTPS_DllAPI const SubscriberQos& get_default_subscriber_qos() const;

    /**
     * This operation retrieves the default value of the Subscriber QoS, that is, the QoS policies which will be used
     * for newly created Subscriber entities in the case where the QoS policies are defaulted in the
     * create_subscriber operation.
     *
     * The values retrieved get_default_subscriber_qos will match the set of values specified on the last successful
     * call to set_default_subscriber_qos, or else, if the call was never made, the default values.
     * @param qos SubscriberQos reference where the default_subscriber_qos is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_default_subscriber_qos(
            SubscriberQos& qos) const;

    /**
     * Fills the SubscriberQos with the values of the XML profile.
     * @param profile_name Subscriber profile name.
     * @param qos SubscriberQos object where the qos is returned.
     * @return RETCODE_OK if the profile exists. RETCODE_BAD_PARAMETER otherwise.
     */
    RTPS_DllAPI ReturnCode_t get_subscriber_qos_from_profile(
            const std::string& profile_name,
            SubscriberQos& qos) const;

    /**
     * This operation sets a default value of the Topic QoS policies which will be used for newly created
     * Topic entities in the case where the QoS policies are defaulted in the create_topic operation.
     *
     * This operation will check that the resulting policies are self consistent; if they are not, the operation
     * will have no effect and return INCONSISTENT_POLICY.
     *
     * The special value TOPIC_QOS_DEFAULT may be passed to this operation to indicate that the default QoS
     * should be reset back to the initial values the factory would use, that is the values that would be used
     * if the set_default_topic_qos operation had never been called.
     * @param qos TopicQos to be set
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    RTPS_DllAPI ReturnCode_t set_default_topic_qos(
            const TopicQos& qos);

    /**
     * This operation retrieves the default value of the Topic QoS, that is, the QoS policies that will be used
     * for newly created Topic entities in the case where the QoS policies are defaulted in the create_topic
     * operation.
     *
     * The values retrieved get_default_topic_qos will match the set of values specified on the last successful
     * call to set_default_topic_qos, or else, TOPIC_QOS_DEFAULT if the call was never made.
     * @return Current default topic qos.
     */
    RTPS_DllAPI const TopicQos& get_default_topic_qos() const;

    /**
     * This operation retrieves the default value of the Topic QoS, that is, the QoS policies that will be used
     * for newly created Topic entities in the case where the QoS policies are defaulted in the create_topic
     * operation.
     *
     * The values retrieved get_default_topic_qos will match the set of values specified on the last successful
     * call to set_default_topic_qos, or else, TOPIC_QOS_DEFAULT if the call was never made.
     * @param qos TopicQos reference where the default_topic_qos is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_default_topic_qos(
            TopicQos& qos) const;

    /**
     * Fills the TopicQos with the values of the XML profile.
     * @param profile_name Topic profile name.
     * @param qos TopicQos object where the qos is returned.
     * @return RETCODE_OK if the profile exists. RETCODE_BAD_PARAMETER otherwise.
     */
    RTPS_DllAPI ReturnCode_t get_topic_qos_from_profile(
            const std::string& profile_name,
            TopicQos& qos) const;


    /* TODO
       bool get_discovered_participants(
            std::vector<fastrtps::rtps::InstanceHandle_t>& participant_handles) const;
     */

    /* TODO
       bool get_discovered_participant_data(
            ParticipantBuiltinTopicData& participant_data,
            const fastrtps::rtps::InstanceHandle_t& participant_handle) const;
     */

    /* TODO
       bool get_discovered_topics(
            std::vector<fastrtps::rtps::InstanceHandle_t>& topic_handles) const;
     */

    /* TODO
       bool get_discovered_topic_data(
            TopicBuiltinTopicData& topic_data,
            const fastrtps::rtps::InstanceHandle_t& topic_handle) const;
     */

    /**
     * This operation checks whether or not the given handle represents an Entity that was created from the
     * DomainParticipant.
     * @param handle InstanceHandle of the entity to look for.
     * @param recursive The containment applies recursively. That is, it applies both to entities
     * (TopicDescription, Publisher, or Subscriber) created directly using the DomainParticipant as well as
     * entities created using a contained Publisher, or Subscriber as the factory, and so forth. (default: true)
     * @return True if entity is contained. False otherwise.
     */
    RTPS_DllAPI bool contains_entity(
            const fastrtps::rtps::InstanceHandle_t& handle,
            bool recursive = true) const;

    /**
     * This operation returns the current value of the time that the service uses to time-stamp data-writes
     * and to set the reception-timestamp for the data-updates it receives.
     * @param current_time Time_t reference where the current time is returned
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t get_current_time(
            fastrtps::Time_t& current_time) const;

    /**
     * This method gives access to a registered type based on its name.
     * @param type_name Name of the type
     * @return TypeSupport corresponding to the type_name
     */
    RTPS_DllAPI TypeSupport find_type(
            const std::string& type_name) const;

    /**
     * Returns the DomainParticipant's handle.
     * @return InstanceHandle of this DomainParticipant.
     */
    RTPS_DllAPI const fastrtps::rtps::InstanceHandle_t& get_instance_handle() const;

    // From here legacy RTPS methods.

    /**
     * @brief Getter for the Participant GUID
     * @return A reference to the GUID
     */
    RTPS_DllAPI const fastrtps::rtps::GUID_t& guid() const;

    /**
     * @brief Getter for the participant names
     * @return Vector with the names
     */
    RTPS_DllAPI std::vector<std::string> get_participant_names() const;

    /**
     * This method can be used when using a StaticEndpointDiscovery mechanism different that the one
     * included in FastRTPS, for example when communicating with other implementations.
     * It indicates the Participant that an Endpoint from the XML has been discovered and
     * should be activated.
     * @param partguid Participant GUID_t.
     * @param userId User defined ID as shown in the XML file.
     * @param kind EndpointKind (WRITER or READER)
     * @return True if correctly found and activated.
     */
    RTPS_DllAPI bool new_remote_endpoint_discovered(
            const fastrtps::rtps::GUID_t& partguid,
            uint16_t userId,
            fastrtps::rtps::EndpointKind_t kind);

    /**
     * @brief Getter for the resource event
     * @return A reference to the resource event
     */
    RTPS_DllAPI fastrtps::rtps::ResourceEvent& get_resource_event() const;

    /**
     * When a DomainParticipant receives an incomplete list of TypeIdentifiers in a
     * PublicationBuiltinTopicData or SubscriptionBuiltinTopicData, it may request the additional type
     * dependencies by invoking the getTypeDependencies operation.
     * @param in TypeIdentifier sequence
     * @return SampleIdentity
     */
    RTPS_DllAPI fastrtps::rtps::SampleIdentity get_type_dependencies(
            const fastrtps::types::TypeIdentifierSeq& in) const;

    /**
     * A DomainParticipant may invoke the operation getTypes to retrieve the TypeObjects associated with a
     * list of TypeIdentifiers.
     * @param in TypeIdentifier sequence
     * @return SampleIdentity
     */
    RTPS_DllAPI fastrtps::rtps::SampleIdentity get_types(
            const fastrtps::types::TypeIdentifierSeq& in) const;

    /**
     * Helps the user to solve all dependencies calling internally to the typelookup service
     * and registers the resulting dynamic type.
     * The registration will be perform asynchronously and the user will be notified through the
     * given callback, which receives the type_name as unique argument.
     * If the type is already registered, the function will return true, but the callback will not be called.
     * If the given type_information is enough to build the type without using the typelookup service,
     * it will return true and the callback will be never called.
     * @param type_information
     * @param type_name
     * @param callback
     * @return true if type is already available (callback will not be called). false if type isn't available yet
     * (the callback will be called if negotiation is success, and ignored in other case).
     */
    RTPS_DllAPI ReturnCode_t register_remote_type(
            const fastrtps::types::TypeInformation& type_information,
            const std::string& type_name,
            std::function<void(const std::string& name, const fastrtps::types::DynamicType_ptr type)>& callback);

    /**
     * @brief Destructor
     */
    RTPS_DllAPI virtual ~DomainParticipant();

    /**
     * @brief Check if the Participant has any Publisher, Subscriber or Topic
     * @return true if any, false otherwise.
     */
    bool has_active_entities();

protected:

    RTPS_DllAPI DomainParticipant(
            const StatusMask& mask = StatusMask::all());

    RTPS_DllAPI DomainParticipant(
            DomainId_t domain_id,
            const DomainParticipantQos& qos,
            DomainParticipantListener* listener,
            const StatusMask& mask = StatusMask::all());

    DomainParticipantImpl* impl_;

    friend class DomainParticipantFactory;

    friend class DomainParticipantImpl;

    friend class ::dds::domain::DomainParticipant;
};

} // namespace dds
} // namespace fastdds
} /* namespace eprosima */

#endif /* _FASTDDS_DOMAIN_PARTICIPANT_HPP_ */
