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

#ifndef FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANT_HPP
#define FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANT_HPP

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/builtin/topic/TopicBuiltinTopicData.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

namespace dds {
namespace domain {
class DomainParticipant;
} // namespace domain
} // namespace dds

namespace eprosima {
namespace fastdds {
namespace rtps {
class ResourceEvent;
} // namespace rtps

namespace dds {
namespace rpc {
class Replier;
class Requester;
class Service;
} // namespace rpc

class DomainParticipantImpl;
class DomainParticipantListener;
class Publisher;
class PublisherQos;
class PublisherListener;
class ReplierQos;
class RequesterQos;
class Subscriber;
class SubscriberQos;
class SubscriberListener;
class TopicQos;

// Not implemented classes
class MultiTopic;

/**
 * Class DomainParticipant used to group Publishers and Subscribers into a single working unit.
 *
 * @ingroup FASTDDS_MODULE
 */
class DomainParticipant : public Entity
{
public:

    /**
     * @brief Destructor
     */
    virtual ~DomainParticipant();

    // Superclass methods

    /**
     * This operation returns the value of the DomainParticipant QoS policies
     *
     * @param qos DomainParticipantQos reference where the qos is going to be returned
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_qos(
            DomainParticipantQos& qos) const;

    /**
     * @brief This operation returns the value of the DomainParticipant QoS policies
     *
     * @return A reference to the DomainParticipantQos
     */
    FASTDDS_EXPORTED_API const DomainParticipantQos& get_qos() const;

    /**
     * This operation sets the value of the DomainParticipant QoS policies.
     *
     * @param qos DomainParticipantQos to be set
     * @return RETCODE_IMMUTABLE_POLICY if any of the Qos cannot be changed, RETCODE_INCONSISTENT_POLICY if the Qos is not
     * self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_qos(
            const DomainParticipantQos& qos) const;

    /**
     * Allows accessing the DomainParticipantListener.
     *
     * @return DomainParticipantListener pointer
     */
    FASTDDS_EXPORTED_API const DomainParticipantListener* get_listener() const;

    /**
     * Modifies the DomainParticipantListener, sets the mask to StatusMask::all()
     *
     * @param listener New value for the DomainParticipantListener
     * @return RETCODE_OK if successful, RETCODE_ERROR otherwise.
     * @warning Do not call this method from a \c DomainParticipantListener callback.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            DomainParticipantListener* listener);

    /**
     * Modifies the DomainParticipantListener, sets the mask to StatusMask::all()
     *
     * @param listener New value for the DomainParticipantListener
     * @param timeout Maximum time to wait for executing callbacks to finish.
     * @return RETCODE_OK if successful, RETCODE_ERROR if failed (timeout expired).
     * @warning Do not call this method from a \c DomainParticipantListener callback.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            DomainParticipantListener* listener,
            const std::chrono::seconds timeout);

    /**
     * Modifies the DomainParticipantListener.
     *
     * @param listener New value for the DomainParticipantListener
     * @param mask StatusMask that holds statuses the listener responds to
     * @return RETCODE_OK if successful, RETCODE_ERROR otherwise.
     * @warning Do not call this method from a \c DomainParticipantListener callback.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            DomainParticipantListener* listener,
            const StatusMask& mask);

    /**
     * Modifies the DomainParticipantListener.
     *
     * @param listener New value for the DomainParticipantListener
     * @param mask StatusMask that holds statuses the listener responds to
     * @param timeout Maximum time to wait for executing callbacks to finish.
     * @return RETCODE_OK if successful, RETCODE_ERROR if failed (timeout expired)
     * @warning Do not call this method from a \c DomainParticipantListener callback.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_listener(
            DomainParticipantListener* listener,
            const StatusMask& mask,
            const std::chrono::seconds timeout);

    /**
     * @brief This operation enables the DomainParticipant
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t enable() override;

    // DomainParticipant specific methods from DDS API

    /**
     * Create a Publisher in this Participant.
     *
     * @param qos QoS of the Publisher.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Publisher.
     */
    FASTDDS_EXPORTED_API Publisher* create_publisher(
            const PublisherQos& qos,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Publisher in this Participant.
     *
     * @param profile_name Publisher profile name.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Publisher.
     */
    FASTDDS_EXPORTED_API Publisher* create_publisher_with_profile(
            const std::string& profile_name,
            PublisherListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Deletes an existing Publisher.
     *
     * @param publisher to be deleted.
     * @return RETCODE_PRECONDITION_NOT_MET if the publisher does not belong to this participant or if it has active DataWriters,
     * RETCODE_OK if it is correctly deleted and RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_publisher(
            const Publisher* publisher);

    /**
     * Create a Subscriber in this Participant.
     *
     * @param qos QoS of the Subscriber.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Subscriber.
     */
    FASTDDS_EXPORTED_API Subscriber* create_subscriber(
            const SubscriberQos& qos,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Subscriber in this Participant.
     *
     * @param profile_name Subscriber profile name.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Subscriber.
     */
    FASTDDS_EXPORTED_API Subscriber* create_subscriber_with_profile(
            const std::string& profile_name,
            SubscriberListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Deletes an existing Subscriber.
     *
     * @param subscriber to be deleted.
     * @return RETCODE_PRECONDITION_NOT_MET if the subscriber does not belong to this participant or if it has active DataReaders,
     * RETCODE_OK if it is correctly deleted and RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_subscriber(
            const Subscriber* subscriber);

    /**
     * Create a Topic in this Participant.
     *
     * @param topic_name Name of the Topic.
     * @param type_name Data type of the Topic.
     * @param qos QoS of the Topic.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Topic.
     */
    FASTDDS_EXPORTED_API Topic* create_topic(
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Create a Topic in this Participant.
     *
     * @param topic_name Name of the Topic.
     * @param type_name Data type of the Topic.
     * @param profile_name Topic profile name.
     * @param listener Pointer to the listener (default: nullptr)
     * @param mask StatusMask that holds statuses the listener responds to (default: all)
     * @return Pointer to the created Topic.
     */
    FASTDDS_EXPORTED_API Topic* create_topic_with_profile(
            const std::string& topic_name,
            const std::string& type_name,
            const std::string& profile_name,
            TopicListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

    /**
     * Deletes an existing Topic.
     *
     * @param topic to be deleted.
     * @return RETCODE_BAD_PARAMETER if the topic passed is a nullptr, RETCODE_PRECONDITION_NOT_MET if the topic does not belong to
     * this participant or if it is referenced by any entity and RETCODE_OK if the Topic was deleted.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_topic(
            const Topic* topic);

    /**
     * Create a ContentFilteredTopic in this Participant.
     *
     * @param name Name of the ContentFilteredTopic
     * @param related_topic Related Topic to being subscribed
     * @param filter_expression Logic expression to create filter
     * @param expression_parameters Parameters to filter content
     * @return Pointer to the created ContentFilteredTopic.
     * @return nullptr if @c related_topic does not belong to this participant.
     * @return nullptr if a topic with the specified @c name has already been created.
     * @return nullptr if a filter cannot be created with the specified @c filter_expression and
     *                 @c expression_parameters.
     */
    FASTDDS_EXPORTED_API ContentFilteredTopic* create_contentfilteredtopic(
            const std::string& name,
            Topic* related_topic,
            const std::string& filter_expression,
            const std::vector<std::string>& expression_parameters);

    /**
     * Create a ContentFilteredTopic in this Participant using a custom filter.
     *
     * @param name Name of the ContentFilteredTopic
     * @param related_topic Related Topic to being subscribed
     * @param filter_expression Logic expression to create filter
     * @param expression_parameters Parameters to filter content
     * @param filter_class_name Name of the filter class to use
     *
     * @return Pointer to the created ContentFilteredTopic.
     * @return nullptr if @c related_topic does not belong to this participant.
     * @return nullptr if a topic with the specified @c name has already been created.
     * @return nullptr if a filter cannot be created with the specified @c filter_expression and
     *                 @c expression_parameters.
     * @return nullptr if the specified @c filter_class_name has not been registered.
     */
    FASTDDS_EXPORTED_API ContentFilteredTopic* create_contentfilteredtopic(
            const std::string& name,
            Topic* related_topic,
            const std::string& filter_expression,
            const std::vector<std::string>& expression_parameters,
            const char* filter_class_name);

    /**
     * Deletes an existing ContentFilteredTopic.
     *
     * @param a_contentfilteredtopic ContentFilteredTopic to be deleted
     * @return RETCODE_BAD_PARAMETER if the topic passed is a nullptr, RETCODE_PRECONDITION_NOT_MET if the topic does not belong to
     * this participant or if it is referenced by any entity and RETCODE_OK if the ContentFilteredTopic was deleted.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_contentfilteredtopic(
            const ContentFilteredTopic* a_contentfilteredtopic);

    /**
     * Create a MultiTopic in this Participant.
     *
     * @param name Name of the MultiTopic
     * @param type_name Result type of the MultiTopic
     * @param subscription_expression Logic expression to combine filter
     * @param expression_parameters Parameters to subscription content
     * @return Pointer to the created ContentFilteredTopic, nullptr in error case
     */
    FASTDDS_EXPORTED_API MultiTopic* create_multitopic(
            const std::string& name,
            const std::string& type_name,
            const std::string& subscription_expression,
            const std::vector<std::string>& expression_parameters);

    /**
     * Deletes an existing MultiTopic.
     *
     * @param a_multitopic MultiTopic to be deleted
     * @return RETCODE_BAD_PARAMETER if the topic passed is a nullptr, RETCODE_PRECONDITION_NOT_MET if the topic does not belong to
     * this participant or if it is referenced by any entity and RETCODE_OK if the Topic was deleted.
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     *
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_multitopic(
            const MultiTopic* a_multitopic);

    /**
     * Gives access to an existing (or ready to exist) enabled Topic.
     * It should be noted that the returned Topic is a local object that acts as a proxy to designate the global
     * concept of topic.
     * Topics obtained by means of find_topic, must also be deleted by means of delete_topic so that the local
     * resources can be released.
     * If a Topic is obtained multiple times by means of find_topic or create_topic, it must also be deleted that same
     * number of times using delete_topic.
     *
     * @param topic_name Topic name
     * @param timeout Maximum time to wait for the Topic
     * @return Pointer to the existing Topic, nullptr in case of error or timeout
     */
    FASTDDS_EXPORTED_API Topic* find_topic(
            const std::string& topic_name,
            const fastdds::dds::Duration_t& timeout);

    /**
     * Create a RPC service.
     *
     * @param service_name Name of the service.
     * @param service_type_name Type name of the service (Request & reply types)
     *
     * @return Pointer to the created service. nullptr in error case.
     */
    FASTDDS_EXPORTED_API rpc::Service* create_service(
            const std::string& service_name,
            const std::string& service_type_name);

    /**
     * Find a RPC service by name
     *
     * @param service_name Name of the service to search for.
     * @return Pointer to the service object if found, nullptr if not found.
     */
    FASTDDS_EXPORTED_API rpc::Service* find_service(
            const std::string& service_name) const;

    /**
     * Delete a registered RPC service
     *
     * @param service Pointer to the service to be deleted.
     * @return RETCODE_OK if the service was deleted, or an specific error code otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_service(
            const rpc::Service* service);

    /**
     * Create a RPC Requester in a given Service.
     *
     * @param service Pointer to a service object where the requester will be created.
     * @param requester_qos QoS of the requester.
     *
     * @return Pointer to the created requester. nullptr in error case.
     */
    FASTDDS_EXPORTED_API rpc::Requester* create_service_requester(
            rpc::Service* service,
            const RequesterQos& requester_qos);

    /**
     * Deletes an existing RPC Requester
     *
     * @param service_name Name of the service where the requester is created.
     * @param requester Pointer to the requester to be deleted.
     * @return RETCODE_OK if the requester was deleted, or an specific error code otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_service_requester(
            const std::string& service_name,
            rpc::Requester* requester);

    /**
     * Create a RPC Replier in a given Service. It will override the current service's replier
     *
     * @param service Pointer to a service object where the Replier will be created.
     * @param replier_qos QoS of the replier.
     *
     * @return Pointer to the created replier. nullptr in error case.
     */
    FASTDDS_EXPORTED_API rpc::Replier* create_service_replier(
            rpc::Service* service,
            const ReplierQos& replier_qos);

    /**
     * Deletes an existing RPC Replier
     *
     * @param service_name Name of the service where the replier is created.
     * @param replier Pointer to the replier to be deleted.
     * @return RETCODE_OK if the replier was deleted, or an specific error code otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_service_replier(
            const std::string& service_name,
            rpc::Replier* replier);

    /**
     * Looks up an existing, locally created @ref TopicDescription, based on its name.
     * May be called on a disabled participant.
     *
     * @param topic_name Name of the @ref TopicDescription to search for.
     * @return Pointer to the topic description, if it has been created locally. Otherwise, nullptr is returned.
     *
     * @remark UNSAFE. It is unsafe to lookup a topic description while another thread is creating a topic.
     */
    FASTDDS_EXPORTED_API TopicDescription* lookup_topicdescription(
            const std::string& topic_name) const;

    /**
     * Allows access to the builtin Subscriber.
     *
     * @return Pointer to the builtin Subscriber, nullptr in error case
     */
    FASTDDS_EXPORTED_API const Subscriber* get_builtin_subscriber() const;

    /**
     * Locally ignore a remote domain participant.
     *
     * @note This action is not reversible.
     *
     * @param handle Identifier of the remote participant to ignore
     * @return RETURN_OK code if everything correct, RETCODE_BAD_PARAMENTER otherwise
     *
     */
    FASTDDS_EXPORTED_API ReturnCode_t ignore_participant(
            const InstanceHandle_t& handle);

    /**
     * Locally ignore a topic.
     *
     * @note This action is not reversible.
     *
     * @param handle Identifier of the topic to ignore
     * @return RETURN_OK code if everything correct, error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     *
     */
    FASTDDS_EXPORTED_API ReturnCode_t ignore_topic(
            const InstanceHandle_t& handle);

    /**
     * Locally ignore a remote datawriter.
     *
     * @note This action is not reversible.
     *
     * @param handle Identifier of the datawriter to ignore
     * @return RETURN_OK code if everything correct, error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     *
     */
    FASTDDS_EXPORTED_API ReturnCode_t ignore_publication(
            const InstanceHandle_t& handle);

    /**
     * Locally ignore a remote datareader.
     *
     * @note This action is not reversible.
     *
     * @param handle Identifier of the datareader to ignore
     * @return RETURN_OK code if everything correct, error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     *
     */
    FASTDDS_EXPORTED_API ReturnCode_t ignore_subscription(
            const InstanceHandle_t& handle);

    /**
     * This operation retrieves the domain_id used to create the DomainParticipant.
     * The domain_id identifies the DDS domain to which the DomainParticipant belongs.
     *
     * @return The Participant's domain_id
     */
    FASTDDS_EXPORTED_API DomainId_t get_domain_id() const;

    /**
     * Deletes all the entities that were created by means of the “create” methods
     *
     * @return RETURN_OK code if everything correct, error code otherwise
     */
    FASTDDS_EXPORTED_API ReturnCode_t delete_contained_entities();

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
     *
     * @return RETCODE_OK if the liveliness was asserted, RETCODE_ERROR otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t assert_liveliness();

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
     *
     * @param qos PublisherQos to be set
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_default_publisher_qos(
            const PublisherQos& qos);

    /**
     * This operation retrieves the default value of the Publisher QoS, that is, the QoS policies which will be used
     * for newly created Publisher entities in the case where the QoS policies are defaulted in the
     * create_publisher operation.
     *
     * The values retrieved get_default_publisher_qos will match the set of values specified on the last successful
     * call to set_default_publisher_qos, or else, if the call was never made, the default values.
     *
     * @return Current default publisher qos.
     */
    FASTDDS_EXPORTED_API const PublisherQos& get_default_publisher_qos() const;

    /**
     * This operation retrieves the default value of the Publisher QoS, that is, the QoS policies which will be used
     * for newly created Publisher entities in the case where the QoS policies are defaulted in the
     * create_publisher operation.
     *
     * The values retrieved get_default_publisher_qos will match the set of values specified on the last successful
     * call to set_default_publisher_qos, or else, if the call was never made, the default values.
     *
     * @param qos PublisherQos reference where the default_publisher_qos is returned
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_publisher_qos(
            PublisherQos& qos) const;

    /**
     * Fills the @ref PublisherQos with the values of the XML profile.
     *
     * @param profile_name Publisher profile name.
     * @param qos @ref PublisherQos object where the qos is returned.
     * @return @ref RETCODE_OK if the profile exists. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_publisher_qos_from_profile(
            const std::string& profile_name,
            PublisherQos& qos) const;

    /**
     * Fills the @ref PublisherQos with the first publisher profile found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref PublisherQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_publisher_qos_from_xml(
            const std::string& xml,
            PublisherQos& qos) const;

    /**
     * Fills the @ref PublisherQos with the publisher profile with \c profile_name to be found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref PublisherQos object where the qos is returned.
     * @param profile_name Publisher profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_publisher_qos_from_xml(
            const std::string& xml,
            PublisherQos& qos,
            const std::string& profile_name) const;

    /**
     * Fills the @ref PublisherQos with the default publisher profile found in the provided XML (if there is).
     *
     * @note This method does not update the default publisher qos (returned by \c get_default_publisher_qos).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref PublisherQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_publisher_qos_from_xml(
            const std::string& xml,
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
     *
     * @param qos SubscriberQos to be set
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_default_subscriber_qos(
            const SubscriberQos& qos);

    /**
     * This operation retrieves the default value of the Subscriber QoS, that is, the QoS policies which will be used
     * for newly created Subscriber entities in the case where the QoS policies are defaulted in the
     * create_subscriber operation.
     *
     * The values retrieved get_default_subscriber_qos will match the set of values specified on the last successful
     * call to set_default_subscriber_qos, or else, if the call was never made, the default values.
     *
     * @return Current default subscriber qos.
     */
    FASTDDS_EXPORTED_API const SubscriberQos& get_default_subscriber_qos() const;

    /**
     * This operation retrieves the default value of the Subscriber QoS, that is, the QoS policies which will be used
     * for newly created Subscriber entities in the case where the QoS policies are defaulted in the
     * create_subscriber operation.
     *
     * The values retrieved get_default_subscriber_qos will match the set of values specified on the last successful
     * call to set_default_subscriber_qos, or else, if the call was never made, the default values.
     *
     * @param qos SubscriberQos reference where the default_subscriber_qos is returned
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_subscriber_qos(
            SubscriberQos& qos) const;

    /**
     * Fills the @ref SubscriberQos with the values of the XML profile.
     *
     * @param profile_name Subscriber profile name.
     * @param qos @ref SubscriberQos object where the qos is returned.
     * @return @ref RETCODE_OK if the profile exists. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_subscriber_qos_from_profile(
            const std::string& profile_name,
            SubscriberQos& qos) const;

    /**
     * Fills the @ref SubscriberQos with the first subscriber profile found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref SubscriberQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_subscriber_qos_from_xml(
            const std::string& xml,
            SubscriberQos& qos) const;

    /**
     * Fills the @ref SubscriberQos with the subscriber profile with \c profile_name to be found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref SubscriberQos object where the qos is returned.
     * @param profile_name Subscriber profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_subscriber_qos_from_xml(
            const std::string& xml,
            SubscriberQos& qos,
            const std::string& profile_name) const;

    /**
     * Fills the @ref SubscriberQos with the default subscriber profile found in the provided XML (if there is).
     *
     * @note This method does not update the default subscriber qos (returned by \c get_default_subscriber_qos).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref SubscriberQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_subscriber_qos_from_xml(
            const std::string& xml,
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
     *
     * @param qos TopicQos to be set
     * @return RETCODE_INCONSISTENT_POLICY if the Qos is not self consistent and RETCODE_OK if the qos is changed correctly.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_default_topic_qos(
            const TopicQos& qos);

    /**
     * This operation retrieves the default value of the Topic QoS, that is, the QoS policies that will be used
     * for newly created Topic entities in the case where the QoS policies are defaulted in the create_topic
     * operation.
     *
     * The values retrieved get_default_topic_qos will match the set of values specified on the last successful
     * call to set_default_topic_qos, or else, TOPIC_QOS_DEFAULT if the call was never made.
     *
     * @return Current default topic qos.
     */
    FASTDDS_EXPORTED_API const TopicQos& get_default_topic_qos() const;

    /**
     * This operation retrieves the default value of the Topic QoS, that is, the QoS policies that will be used
     * for newly created Topic entities in the case where the QoS policies are defaulted in the create_topic
     * operation.
     *
     * The values retrieved get_default_topic_qos will match the set of values specified on the last successful
     * call to set_default_topic_qos, or else, TOPIC_QOS_DEFAULT if the call was never made.
     *
     * @param qos TopicQos reference where the default_topic_qos is returned
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_topic_qos(
            TopicQos& qos) const;

    /**
     * Fills the @ref TopicQos with the values of the XML profile.
     *
     * @param profile_name Topic profile name.
     * @param qos @ref TopicQos object where the qos is returned.
     * @return @ref RETCODE_OK if the profile exists. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_topic_qos_from_profile(
            const std::string& profile_name,
            TopicQos& qos) const;

    /**
     * Fills the @ref TopicQos with the values of the XML profile, and also its corresponding topic and data type names (if specified).
     *
     * @param profile_name Topic profile name.
     * @param qos @ref TopicQos object where the qos is returned.
     * @param topic_name String where the name of the topic associated to this profile is returned (if specified).
     * @param topic_data_type String where the name of the topic data type associated to this profile is returned (if specified).
     * @return @ref RETCODE_OK if the profile exists. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_topic_qos_from_profile(
            const std::string& profile_name,
            TopicQos& qos,
            std::string& topic_name,
            std::string& topic_data_type) const;

    /**
     * Fills the @ref TopicQos with the first topic profile found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref TopicQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos) const;

    /**
     * Fills the @ref TopicQos with the first topic profile found in the provided XML, and also its corresponding topic and data type names (if specified).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref TopicQos object where the qos is returned.
     * @param topic_name String where the name of the topic associated to this profile is returned (if specified).
     * @param topic_data_type String where the name of the topic data type associated to this profile is returned (if specified).
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos,
            std::string& topic_name,
            std::string& topic_data_type) const;

    /**
     * Fills the @ref TopicQos with the topic profile with \c profile_name to be found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref TopicQos object where the qos is returned.
     * @param profile_name Topic profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos,
            const std::string& profile_name) const;

    /**
     * Fills the @ref TopicQos with the topic profile with \c profile_name to be found in the provided XML, and also its corresponding topic and data type names (if specified).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref TopicQos object where the qos is returned.
     * @param topic_name String where the name of the topic associated to this profile is returned (if specified).
     * @param topic_data_type String where the name of the topic data type associated to this profile is returned (if specified).
     * @param profile_name Topic profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos,
            std::string& topic_name,
            std::string& topic_data_type,
            const std::string& profile_name) const;

    /**
     * Fills the @ref TopicQos with the default topic profile found in the provided XML (if there is).
     *
     * @note This method does not update the default topic qos (returned by \c get_default_topic_qos).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref TopicQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos) const;

    /**
     * Fills the @ref TopicQos with the default topic profile found in the provided XML (if there is), and also its corresponding topic and data type names (if specified).
     *
     * @note This method does not update the default topic qos (returned by \c get_default_topic_qos).
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref TopicQos object where the qos is returned.
     * @param topic_name String where the name of the topic associated to this profile is returned (if specified).
     * @param topic_data_type String where the name of the topic data type associated to this profile is returned (if specified).
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_topic_qos_from_xml(
            const std::string& xml,
            TopicQos& qos,
            std::string& topic_name,
            std::string& topic_data_type) const;

    /**
     * Fills the @ref ReplierQos with the values of the XML profile.
     *
     * @param profile_name Replier profile name.
     * @param qos @ref ReplierQos object where the qos is returned.
     * @return @ref RETCODE_OK if the profile exists. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_replier_qos_from_profile(
            const std::string& profile_name,
            ReplierQos& qos) const;

    /**
     * Fills the @ref ReplierQos with the first replier profile found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref ReplierQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_replier_qos_from_xml(
            const std::string& xml,
            ReplierQos& qos) const;

    /**
     * Fills the @ref ReplierQos with the replier profile with \c profile_name to be found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref ReplierQos object where the qos is returned.
     * @param profile_name Replier profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_replier_qos_from_xml(
            const std::string& xml,
            ReplierQos& qos,
            const std::string& profile_name) const;

    /**
     * Fills the @ref ReplierQos with the default replier profile found in the provided XML (if there is).
     *
     * @note This method does not update the default replier qos.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref ReplierQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_replier_qos_from_xml(
            const std::string& xml,
            ReplierQos& qos) const;

    /**
     * Fills the @ref RequesterQos with the values of the XML profile.
     *
     * @param profile_name Requester profile name.
     * @param qos @ref RequesterQos object where the qos is returned.
     * @return @ref RETCODE_OK if the profile exists. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_requester_qos_from_profile(
            const std::string& profile_name,
            RequesterQos& qos) const;

    /**
     * Fills the @ref RequesterQos with the first requester profile found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref RequesterQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_requester_qos_from_xml(
            const std::string& xml,
            RequesterQos& qos) const;

    /**
     * Fills the @ref RequesterQos with the requester profile with \c profile_name to be found in the provided XML.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref RequesterQos object where the qos is returned.
     * @param profile_name Requester profile name.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_requester_qos_from_xml(
            const std::string& xml,
            RequesterQos& qos,
            const std::string& profile_name) const;

    /**
     * Fills the @ref RequesterQos with the default requester profile found in the provided XML (if there is).
     *
     * @note This method does not update the default requester qos.
     *
     * @param xml Raw XML string containing the profile to be used to fill the \c qos structure.
     * @param qos @ref RequesterQos object where the qos is returned.
     * @return @ref RETCODE_OK on success. @ref RETCODE_BAD_PARAMETER otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_default_requester_qos_from_xml(
            const std::string& xml,
            RequesterQos& qos) const;

    /**
     * Retrieves the list of DomainParticipants that have been discovered in the domain and are not "ignored".
     *
     * @param [out] participant_handles Reference to the vector where discovered participants will be returned
     * @return RETCODE_OK if everything correct, error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_discovered_participants(
            std::vector<InstanceHandle_t>& participant_handles) const;

    /**
     * Retrieves the DomainParticipant data of a discovered not ignored participant.
     *
     * @param [out] participant_data Reference to the ParticipantBuiltinTopicData object to return the data
     * @param participant_handle InstanceHandle of DomainParticipant to retrieve the data from
     * @return RETCODE_OK if everything correct, PRECONDITION_NOT_MET if participant does not exist
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_discovered_participant_data(
            ParticipantBuiltinTopicData& participant_data,
            const InstanceHandle_t& participant_handle) const;

    /**
     * Retrieves the list of topics that have been discovered in the domain and are not "ignored".
     *
     * @param [out] topic_handles Reference to the vector where discovered topics will be returned
     * @return RETCODE_OK if everything correct, error code otherwise
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_discovered_topics(
            std::vector<InstanceHandle_t>& topic_handles) const;

    /**
     * Retrieves the Topic data of a discovered not ignored topic.
     *
     * @param [out] topic_data Reference to the TopicBuiltinTopicData object to return the data
     * @param topic_handle InstanceHandle of Topic to retrieve the data from
     * @return RETCODE_OK if everything correct, PRECONDITION_NOT_MET if topic does not exist
     *
     * @warning Not supported yet. Currently returns RETCODE_UNSUPPORTED
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_discovered_topic_data(
            builtin::TopicBuiltinTopicData& topic_data,
            const InstanceHandle_t& topic_handle) const;

    /**
     * This operation checks whether or not the given handle represents an Entity that was created from the
     * DomainParticipant.
     *
     * @param a_handle InstanceHandle of the entity to look for.
     * @param recursive The containment applies recursively. That is, it applies both to entities
     * (TopicDescription, Publisher, or Subscriber) created directly using the DomainParticipant as well as
     * entities created using a contained Publisher, or Subscriber as the factory, and so forth. (default: true)
     * @return True if entity is contained. False otherwise.
     */
    FASTDDS_EXPORTED_API bool contains_entity(
            const InstanceHandle_t& a_handle,
            bool recursive = true) const;

    /**
     * This operation returns the current value of the time that the service uses to time-stamp data-writes
     * and to set the reception-timestamp for the data-updates it receives.
     *
     * @param current_time Time_t reference where the current time is returned
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_current_time(
            fastdds::dds::Time_t& current_time) const;

    // DomainParticipant methods specific from Fast DDS

    /**
     * Register a type in this participant.
     *
     * @param type TypeSupport.
     * @param type_name The name that will be used to identify the Type.
     * @return RETCODE_BAD_PARAMETER if the size of the name is 0, RERCODE_PRECONDITION_NOT_MET if there is another TypeSupport
     * with the same name and RETCODE_OK if it is correctly registered.
     */
    FASTDDS_EXPORTED_API ReturnCode_t register_type(
            TypeSupport type,
            const std::string& type_name);

    /**
     * Register a type in this participant.
     *
     * @param type TypeSupport.
     * @return RETCODE_BAD_PARAMETER if the size of the name is 0, RERCODE_PRECONDITION_NOT_MET if there is another TypeSupport
     * with the same name and RETCODE_OK if it is correctly registered.
     */
    FASTDDS_EXPORTED_API ReturnCode_t register_type(
            TypeSupport type);

    /**
     * Unregister a type in this participant.
     *
     * @param typeName Name of the type
     * @return RETCODE_BAD_PARAMETER if the size of the name is 0, RERCODE_PRECONDITION_NOT_MET if there are entities using that
     * TypeSupport and RETCODE_OK if it is correctly unregistered.
     */
    FASTDDS_EXPORTED_API ReturnCode_t unregister_type(
            const std::string& typeName);

    /**
     * This method gives access to a registered type based on its name.
     *
     * @param type_name Name of the type
     * @return TypeSupport corresponding to the type_name
     */
    FASTDDS_EXPORTED_API TypeSupport find_type(
            const std::string& type_name) const;

    /**
     * Register a service type in this participant.
     *
     * @param service_type ServiceTypeSupport.
     * @param service_type_name The name that will be used to identify the service type.
     * @return RETCODE_OK if it is correctly registered. Error code otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t register_service_type(
            rpc::ServiceTypeSupport service_type,
            const std::string& service_type_name);

    /**
     * Unregister a service type in this participant.
     *
     * @param service_type_name Name of the type
     * @return RETCODE_OK if it is correctly unregistered. Error code otherwise.
     */
    FASTDDS_EXPORTED_API ReturnCode_t unregister_service_type(
            const std::string& service_type_name);

    /**
     * This method gives access to a registered service type based on its name.
     *
     * @param service_type_name Name of the type
     * @return ServiceTypeSupport corresponding to the service_type_name
     */
    FASTDDS_EXPORTED_API rpc::ServiceTypeSupport find_service_type(
            const std::string& service_type_name) const;

    /**
     * Returns the DomainParticipant's handle.
     *
     * @return InstanceHandle of this DomainParticipant.
     */
    FASTDDS_EXPORTED_API const InstanceHandle_t& get_instance_handle() const;

    // From here legacy RTPS methods.

    /**
     * @brief Getter for the Participant GUID
     *
     * @return A reference to the GUID
     */
    FASTDDS_EXPORTED_API const fastdds::rtps::GUID_t& guid() const;

    /**
     * @brief Getter for the participant names
     *
     * @return Vector with the names
     */
    FASTDDS_EXPORTED_API std::vector<std::string> get_participant_names() const;

    /**
     * This method can be used when using a StaticEndpointDiscovery mechanism different that the one
     * included in Fast DDS, for example when communicating with other implementations.
     * It indicates the Participant that an Endpoint from the XML has been discovered and
     * should be activated.
     *
     * @param partguid Participant GUID_t.
     * @param userId User defined ID as shown in the XML file.
     * @param kind EndpointKind (WRITER or READER)
     * @return True if correctly found and activated.
     */
    FASTDDS_EXPORTED_API bool new_remote_endpoint_discovered(
            const fastdds::rtps::GUID_t& partguid,
            uint16_t userId,
            fastdds::rtps::EndpointKind_t kind);

    /**
     * Register a custom content filter factory, which can be used to create a ContentFilteredTopic.
     *
     * DDS specifies a SQL-like content filter to be used by content filtered topics.
     * If this filter does not meet your filtering requirements, you can register a custom filter factory.
     *
     * To use a custom filter, a factory for it must be registered in the following places:
     *
     * - In any application that uses the custom filter factory to create a ContentFilteredTopic and the corresponding
     *   DataReader.
     *
     * - In each application that writes the data to the applications mentioned above.
     *
     * For example, suppose Application A on the subscription side creates a Topic named X and a ContentFilteredTopic
     * named filteredX (and a corresponding DataReader), using a previously registered content filter factory, myFilterFactory.
     * With only that, you will have filtering at the subscription side.
     * If you also want to perform filtering in any application that publishes Topic X, then you also need to register
     * the same definition of the ContentFilterFactory myFilterFactory in that application.
     *
     * Each @c filter_class_name can only be used to register a content filter factory once per DomainParticipant.
     *
     * @param filter_class_name Name of the filter class. Cannot be nullptr, must not exceed 255 characters, and must
     *                          be unique within this DomainParticipant.
     * @param filter_factory    Factory of content filters to be registered. Cannot be nullptr.
     *
     * @return RETCODE_BAD_PARAMETER if any parameter is nullptr, or the filter_class_name exceeds 255 characters.
     * @return RETCODE_PRECONDITION_NOT_MET if the filter_class_name has been already registered.
     * @return RETCODE_PRECONDITION_NOT_MET if filter_class_name is FASTDDS_SQLFILTER_NAME.
     * @return RETCODE_OK if the filter is correctly registered.
     */
    FASTDDS_EXPORTED_API ReturnCode_t register_content_filter_factory(
            const char* filter_class_name,
            IContentFilterFactory* const filter_factory);

    /**
     * Lookup a custom content filter factory previously registered with register_content_filter_factory.
     *
     * @param filter_class_name Name of the filter class. Cannot be nullptr.
     *
     * @return nullptr if the given filter_class_name has not been previously registered on this DomainParticipant.
     *         Otherwise, the content filter factory previously registered with the given filter_class_name.
     */
    FASTDDS_EXPORTED_API IContentFilterFactory* lookup_content_filter_factory(
            const char* filter_class_name);

    /**
     * Unregister a custom content filter factory previously registered with register_content_filter_factory.
     *
     * A filter_class_name can be unregistered only if it has been previously registered to the DomainParticipant with
     * register_content_filter_factory.
     *
     * The unregistration of filter is not allowed if there are any existing ContentFilteredTopic objects that are
     * using the filter.
     *
     * If there is any existing discovered DataReader with the same filter_class_name, filtering on the writer side will be
     * stopped, but this operation will not fail.
     *
     * @param filter_class_name Name of the filter class. Cannot be nullptr.
     *
     * @return RETCODE_BAD_PARAMETER if the filter_class_name is nullptr.
     * @return RERCODE_PRECONDITION_NOT_MET if the filter_class_name has not been previously registered.
     * @return RERCODE_PRECONDITION_NOT_MET if there is any ContentFilteredTopic referencing the filter.
     * @return RETCODE_OK if the filter is correctly unregistered.
     */
    FASTDDS_EXPORTED_API ReturnCode_t unregister_content_filter_factory(
            const char* filter_class_name);

    /**
     * @brief Check if the Participant has any Publisher, Subscriber or Topic
     *
     * @return true if any, false otherwise.
     */
    FASTDDS_EXPORTED_API bool has_active_entities();

protected:

    DomainParticipant(
            const StatusMask& mask = StatusMask::all());

    DomainParticipantImpl* impl_;

    friend class DomainParticipantFactory;

    friend class DomainParticipantImpl;

    friend class ::dds::domain::DomainParticipant;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_DOMAIN__DOMAINPARTICIPANT_HPP
