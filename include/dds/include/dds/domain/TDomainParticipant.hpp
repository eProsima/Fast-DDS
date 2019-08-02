#ifndef OMG_TDDS_DOMAIN_DOMAIN_PARTICIPANT_HPP_
#define OMG_TDDS_DOMAIN_DOMAIN_PARTICIPANT_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>

#include <dds/core/detail/conformance.hpp>
#include <dds/core/types.hpp>
#include <dds/core/Time.hpp>
#include <dds/core/Entity.hpp>
#include <dds/domain/qos/DomainParticipantQos.hpp>

#include <dds/topic/qos/TopicQos.hpp>

#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>


namespace dds
{
namespace domain
{

template <typename DELEGATE>
class TDomainParticipant;

class DomainParticipantListener;
}
}

/**
 * @brief
 * A DomainParticipant represents the local membership of the application in a
 * Domain.
 *
 * The DomainParticipant represents the participation of the application on
 * a communication plane that isolates applications running on the same
 * set of physical computers from each other. A domain establishes a virtual
 * network linking all applications that share the same domainId and isolating
 * them from applications running on different domains. In this way, several
 * independent distributed applications can coexist in the same physical
 * network without interfering, or even being aware of each other.
 *
 * @see @ref DCPS_Modules_DomainParticipant "Domain Participant"
 */
template <typename DELEGATE>
class dds::domain::TDomainParticipant : public ::dds::core::TEntity<DELEGATE>
{
public:
    /**
     * Local representation of the dds::domain::DomainParticipantListener
     */
    typedef dds::domain::DomainParticipantListener Listener;

public:
    OMG_DDS_REF_TYPE_PROTECTED_DC(TDomainParticipant, dds::core::TEntity, DELEGATE)
    OMG_DDS_EXPLICIT_REF_BASE(TDomainParticipant, dds::core::Entity)

public:
    /**
     * Creates a new DomainParticipant object. The DomainParticipant signifies
     * that the calling application intends to join the Domain identified by
     * the domain_id argument.
     *
     * The DomainParticipant will be created with the QoS values specified on the last
     * successful call to
     * @link dds::domain::DomainParticipant::default_participant_qos(const ::dds::domain::qos::DomainParticipantQos& qos)
     * DomainParticipant::default_publisher_qos(qos) @endlink or, if the call was never
     * made, the @ref anchor_dds_domain_domainparticipant_qos_defaults "default" values.
     *
     * @param id the id of the domain joined by the new DomainParticipant
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    TDomainParticipant(uint32_t id);

    /**
     * Creates a new DomainParticipant object. The DomainParticipant signifies
     * that the calling application intends to join the Domain identified by
     * the domain_id argument.
     *
     * The DomainParticipant will be created with the DomainParticipantQos
     * passed as an argument.
     *
     * @param id the id of the domain joined by the new DomainParticipant
     * @param qos the QoS settings for the new DomainParticipant
     * @param listener the listener
     * @param event_mask the mask defining the events for which the listener
     *                  will be notified.
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    TDomainParticipant(uint32_t                                        id,
                       const dds::domain::qos::DomainParticipantQos&   qos,
                       dds::domain::DomainParticipantListener*         listener = NULL,
                       const dds::core::status::StatusMask&            event_mask = dds::core::status::StatusMask::none());

public:
    /** @cond */
    virtual ~TDomainParticipant();
    /** @endcond */

public:

    /**
     * Register a listener with the DomainParticipant.
     *
     * The notifications received by the listener depend on the
     * status mask with which it was registered.
     *
     * Listener un-registration is performed by setting the listener to NULL.
     *
     * See also @ref DCPS_Modules_Infrastructure_Listener "listener information".
     *
     * @param listener the listener
     * @param event_mask the mask defining the events for which the listener
     *                    will be notified.
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::UnsupportedError
     *                  A status was selected that cannot be supported because
     *                  the infrastructure does not maintain the required connectivity information.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    void listener(Listener* listener,
                  const ::dds::core::status::StatusMask& event_mask);

    /**
     * Get the listener of this DomainParticipant.
     *
     * See also @ref DCPS_Modules_Infrastructure_Listener "listener information".
     *
     * @return the listener
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     */
    Listener* listener() const;

    /**
     * Gets the DomainParticipantQos setting for this instance.
     *
     * @return the qos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    const dds::domain::qos::DomainParticipantQos& qos() const;

    /**
     * Sets the DomainParticipantQos setting for this instance.
     *
     * @param qos the qos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    void qos(const dds::domain::qos::DomainParticipantQos& qos);

    /**
     * This operation retrieves the domain_id used to create the
     * DomainParticipant. The domain_id identifies the DDS domain
     * to which the DomainParticipant belongs.
     *
     * Each DDS domain represents a separate data communication
     * plane isolated from other domains.
     *
     * @return the domain id
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     */
    uint32_t domain_id() const;


    /**
     * This operation will manually assert the liveliness for the DomainParticipant.
     *
     * This way, the Data Distribution Service is informed that the DomainParticipant
     * is still alive. This operation only needs to be used when the DomainParticipant
     * contains DataWriters with the dds:core::policy::LivelinessQosPolicy::ManualByParticipant(),
     * and it will only affect the liveliness of those DataWriters.
     *
     * Writing data via the write operation of a DataWriter will assert the liveliness on
     * the DataWriter itself and its DomainParticipant. Therefore,
     * assert_liveliness is only needed when not writing regularly.
     * The liveliness should be asserted by the application, depending on the
     * LivelinessQosPolicy.
     *
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     */
    void assert_liveliness();


    /**
     * This operation checks whether or not the given handle represents
     * an Entity that was created by using this DomainParticipant.
     *
     * The containment applies recursively. That is, it applies both to
     * entities (TopicDescription, Publisher, or Subscriber) created directly
     * using the DomainParticipant as well as entities created using a
     * contained Publisher, or Subscriber as the factory, and so forth.
     *
     * @param  handle   the instance handle for which the containement
     *                  relationship has to be checked
     * @return true     if the handle belongs to an Entity belonging
     *                  to this DomainParticipant
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     */
    bool contains_entity(const ::dds::core::InstanceHandle& handle);

    /**
     * This operation returns the current value of the time that the service
     * uses to time-stamp data writes and to set the reception timestamp
     * for the data updates it receives.
     *
     * @return the current time
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     */
    dds::core::Time current_time() const;

    /** @copydoc dds::domain::DomainParticipant::qos(const dds::domain::qos::DomainParticipantQos& qos) */
    TDomainParticipant& operator << (const dds::domain::qos::DomainParticipantQos& qos);

    /** @copydoc dds::domain::DomainParticipant::qos() */
    const TDomainParticipant& operator >> (dds::domain::qos::DomainParticipantQos& qos) const;

public:
    /**
     * Gets the default DomainParticipantQos.
     *
     * This operation gets an object with the default global DomainParticipant
     * QosPolicy settings which is used for newly
     * created DomainParticipant objects, in case no QoS was provided during the creation.
     *
     * The values retrieved by this operation match the set of values specified on the last
     * successful call to
     * dds::domain::DomainParticipant::default_participant_qos(const ::dds::domain::qos::DomainParticipantQos& qos),
     * or, if the call was never made, the @ref anchor_dds_domain_domainparticipant_qos_defaults "default" values.
     *
     * @return the default DomainParticipantQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    static dds::domain::qos::DomainParticipantQos default_participant_qos();

    /**
     * Sets the default DomainParticipantQos.
     *
     * This QoS will be used by all following DomainParticipant creations when no
     * QoS was given during those creations or the QoS is given that was returned
     * by dds::domain::DomainParticipant::default_participant_qos().
     *
     * @param qos the default DomainParticipantQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    static void default_participant_qos(const ::dds::domain::qos::DomainParticipantQos& qos);

    /**
     * Gets the default PublisherQos of the DomainParticipant.
     *
     * This operation gets an object with the default Publisher QosPolicy settings of
     * the DomainParticipant (that is the PublisherQos) which is used for newly
     * created Publisher objects, in case no QoS was provided during the creation.
     *
     * The values retrieved by this operation match the set of values specified on the last
     * successful call to
     * dds::domain::DomainParticipant::default_publisher_qos(const ::dds::pub::qos::PublisherQos& qos),
     * or, if the call was never made, the @ref anchor_dds_pub_publisher_qos_defaults "default" values.
     *
     * @return the default PublisherQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    dds::pub::qos::PublisherQos default_publisher_qos() const;

    /**
     * Sets the default PublisherQos of the DomainParticipant.
     *
     * This operation sets the default PublisherQos of the DomainParticipant which
     * is used for newly created Publisher objects, when no QoS is provided.
     *
     * The PublisherQos is always self consistent, because its policies do not depend on each
     * other. This means that this operation never throws dds::core::InconsistentPolicyError.
     *
     * The values set by this operation are returned by dds::domain::DomainParticipant::default_publisher_qos().
     *
     * @param qos the default PublisherQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::UnsupportedError
     *                  One or more of the selected QosPolicy values are
     *                  currently not supported by OpenSplice.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    TDomainParticipant& default_publisher_qos(const ::dds::pub::qos::PublisherQos& qos);

    /**
     * Gets the default SubscriberQos of the DomainParticipant.
     *
     * This operation gets an object with the default Subscriber QosPolicy settings of
     * the DomainParticipant (that is the SubscriberQos) which is used for newly
     * created Subscriber objects, in case no QoS was provided during the creation.
     *
     * The values retrieved by this operation match the set of values specified on the last
     * successful call to
     * dds::domain::DomainParticipant::default_subscriber_qos(const :dds::sub::qos::SubscriberQos& qos),
     * or, if the call was never made, the @ref anchor_dds_sub_subscriber_qos_defaults "default" values.
     *
     * @return the default SubscriberQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    dds::sub::qos::SubscriberQos default_subscriber_qos() const;

    /**
     * Sets the default SubscriberQos of the DomainParticipant.
     *
     * This operation sets the default SubscriberQos of the DomainParticipant which
     * is used for newly created Subscriber objects, when no QoS is provided.
     *
     * The SubscriberQos is always self consistent, because its policies do not depend on each
     * other. This means that this operation never throws dds::core::InconsistentPolicyError.
     *
     * The values set by this operation are returned by dds::domain::DomainParticipant::default_subscriber_qos().
     *
     * @param qos the default SubscriberQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::UnsupportedError
     *                  One or more of the selected QosPolicy values are
     *                  currently not supported by OpenSplice.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    TDomainParticipant& default_subscriber_qos(const ::dds::sub::qos::SubscriberQos& qos);

    /**
     * Gets the default TopicQos of the DomainParticipant.
     *
     * This operation gets an object with the default Topic QosPolicy settings of
     * the DomainParticipant (that is the TopicQos) which is used for newly
     * created Topic objects, in case no QoS was provided during the creation.
     *
     * The values retrieved by this operation match the set of values specified on the last
     * successful call to
     * dds::domain::DomainParticipant::default_topic_qos(const dds::topic::qos::TopicQos& qos),
     * or, if the call was never made, the @ref anchor_dds_topic_qos_defaults "default" values.
     *
     * @return the default TopicQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    dds::topic::qos::TopicQos default_topic_qos() const;

    /**
     * Sets the default TopicQos of the DomainParticipant.
     *
     * This operation sets the default SubscriberQos of the DomainParticipant which
     * is used for newly created Subscriber objects, when no QoS is provided.
     *
     * This operation checks if the TopicQos is self consistent. If it is not, the
     * operation has no effect and throws dds::core::InconsistentPolicyError.
     *
     * The values set by this operation are returned by dds::domain::DomainParticipant::default_topic_qos().
     *
     * @param qos the default TopicQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::UnsupportedError
     *                  One or more of the selected QosPolicy values are
     *                  currently not supported by OpenSplice.
     * @throws dds::core::InconsistentPolicyError
     *                  The parameter qos contains conflicting QosPolicy settings,
     *                  e.g. a history depth that is higher than the specified resource limits.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    TDomainParticipant& default_topic_qos(const dds::topic::qos::TopicQos& qos);

#ifdef DOXYGEN_FOR_ISOCPP2
     /*
      * The above macro is never (and must never) be defined in normal compilation.
      *
      * The following code is for documenting proprietary API only.
      */

    /**
      * This operation will create a snapshot of all persistent data matching the provided
      * partition and topic expressions and store the snapshot at the location indicated by
      * the URI. Only persistent data available on the local node is considered.
      *
      * @note This is a proprietary OpenSplice extension.
      *
      *<b>Detailed Description</b><br>
      *
      * This operation will create a snapshot of all persistent data matching the provided
      * partition and topic expressions and store the snapshot at the location indicated by
      * the URI. Only persistent data available on the local node is considered. This
      * operation will fire an event to trigger the snapshot creation by the durability service
      * and then return while the durability service fulfills the snapshot request.
      * The created snapshot can then be used as the persistent store for the durability
      * service next time it starts up by configuring the location of the snapshot as the
      * persistent store in the configuration file. The durability service will then use the
      * snapshot as the regular store (and can thus also alter its contents).
      *
      * <i>Call</i><br>
      * This is a proprietary operation and can be called by using the operator->.
      * @code{.cpp}
      * dds::domain::DomainParticipant dp(domainId);
      * dp->create_persistent_snapshot(...);
      * @endcode
      *
      * @param partition_expression The expression of all partitions involved in
      *                             the snapshot; this may contain wildcards.
      * @param topic_expression     The expression of all topics involved in
      *                             the snapshot; this may contain wildcards.
      * @param uri                  The location where to store the snapshot.
      *                             Currently only directories are supported.
      */
    void create_persistent_snapshot(const std::string& partition_expression, const std::string& topic_expression, const std::string& uri);

#endif

#ifdef DOXYGEN_FOR_ISOCPP2
     /*
      * The above macro is never (and must never) be defined in normal compilation.
      *
      * The following code is for documenting proprietary API only.
      */

    /**
      * This operation safely detaches the application from all domains it is currently
      * participating in.
      *
      * @note This is a proprietary OpenSplice extension.
      *
      *<b>Detailed Description</b><br>
      *
      * This operation safely detaches the application from all domains it is currently
      * participating in. When this operation has been performed successfully,
      * the application is no longer connected to any Domain.
      * For Federated domains finishing this operation successfully means that all shared
      * memory segments have been safely un-mapped from the application process.
      * For SingleProcess mode domains this means all services for all domains have been
      * stopped. This allows graceful termination of the OSPL services that run as threads
      * within the application. Graceful termination of services in this mode would for
      * instance allow durability flushing of persistent data and networking termination
      * announcement over the network.
      * When this call returns further access to all domains will be denied and it will
      * not be possible for the application to open or re-open any DDS domain.
      * <p>
      * The behavior of the detach_all_domains operation is determined by the block_operations
      * and delete_entities parameters:<br>
      * <dl>
      * <dt>block_operations:</dt>
      * <dd>This parameter specifies if the application wants any DDS operation to be blocked
      *     or not while detaching. When true, any DDS operation called during this operation
      *     will be blocked and remain blocked forever (so also after the detach operation has
      *     completed and returns to the caller). When false, any DDS operation called during
      *     this operation may return RETCODE_ALREADY_DELETED. Please note that a listener
      *     callback is not considered an operation in progress. Of course, if a DDS operation
      *     is called from within the listener callback, that operation will be blocked
      *     during the detaching if this attribute is set to TRUE.
      * </dd>
      * <dt>delete_entities:</dt>
      * <dd>This parameter specifies if the application wants the DDS entities created by
      *     the application to be deleted (synchronously) while detaching from the domain or
      *     not. If true, all application entities are guaranteed to be deleted when the call
      *     returns. If false, application entities will not explicitly be deleted by this
      *     operation. In case of federated mode, the splice-daemon will delete them
      *     asynchronously after this operation has returned. In case of SingleProcess mode
      *     this attribute is ignored and clean up will always be performed, as this cannot
      *     be delegated to a different process.
      * </dd>
      * </dl>
      * </p>
      * @note In federated mode when the detach_all_domain operation is called with
      * block_operations is false and delete_entities is false then the DDS operations
      * which are in progress and which are waiting for some condition to become true
      * or waiting for an event to occur while the detach operation is performed may be
      * blocked.
      * <br>
      *
      * <i>Call</i><br>
      * This is a proprietary operation and can be called by using the operator->.
      * @code{.cpp}
      * dds::domain::DomainParticipant dp(domainId);
      * dp->detach_all_domains(...);
      *
      * or use
      *
      * org::opensplice::domain::DomainParticipant::detach_all_domains(...)
      * @endcode
      *
      * @param block_operations Indicates whether the application wants any operations that
      *                         are called while detaching to be blocked or not.
      * @param delete_entities  Indicates whether the application DDS entities in the 'connected'
      *                         domains must be deleted synchronously during detaching.
      */
    static void detach_all_domains(bool block_operations, bool delete_entities);


#endif
    
#ifdef DOXYGEN_FOR_ISOCPP2
     /*
      * The above macro is never (and must never) be defined in normal compilation.
      *
      * The following code is for documenting proprietary API only.
      */
    /**
      * This operation set a property in the domainparticipant
      *
      * @note This is a proprietary OpenSplice extension.
      *
      *<b>Detailed Description</b><br>
      *
      * This operation sets a property in the domain participant to the specified value.
      * <p>
      * Currently, the following properties are defined:
      * <i>isolateNode</i> : The isolateNode property allows applications to isolate the federation from the 
      *                  rest of the Domain, i.e. at network level disconnect the node from the rest of the 
      *                  system. Additionally, they also need to be able to issue a request to reconnect 
      *                  their federation to the domain again after which the durability merge-policy 
      *                  that is configured needs to be applied. 
      *                  To isolate a federation, the application needs to set the isolateNode property 
      *                  value to ‘true’ and to (de)isolate the federation the same property needs to set to ‘false’. 
      *                  The default value of the isolateNode property is ‘false’.       
      *                  All data that is published after isolateNode is set to true will not be sent to the network and 
      *                  any data received from the network will be ignored. 
      *                  Be aware that data being processed by the network service at time of isolating a node may still 
      *                  be sent to the network due to asynchronous nature of network service internals.
      *                  The value is interpreted as a boolean (i.e., it must be either ‘true’ or ‘false’).
      *                  <i>false</i> (default): The federation is connected to the domain.
      *                  <i>true</i>: The federation is disconnected from the domain meaning that data is not published 
      *                   on the network and data from the network is ignored.
      * </p>
      * <br>
      *
      * <i>Call</i><br>
      * This is a proprietary operation and can be called by using the operator->.
      * @code{.cpp}
      * dds::domain::DomainParticipant dp(domainId);
      * dp->set_property(...);
      * @endcode
      *
      * @param name  Indicates the name of the property to be set
      * @param value Indicates the value to set
      * @throws dds::core::Error
      *                  An internal error has occurred.
      * @throws dds::core::InvalidArgumentError
      *                  an invalid value has been specified.
      * @throws dds::core::AlreadyClosedError
      *                  The entity has already been closed.
      * @throws dds::core::OutOfResourcesError
      *                  The Data Distribution Service ran out of resources to
      *                  complete this operation.
      * @throws dds::core::UnsupportedError
      *                  The name specifies an undefined property or the operation is not supported in this version.
      */
    void set_property(std::string property, std::string value);
#endif 

#ifdef DOXYGEN_FOR_ISOCPP2
     /*
      * The above macro is never (and must never) be defined in normal compilation.
      *
      * The following code is for documenting proprietary API only.
      */
    /**
      * This operation get a property from the domainparticipant
      *
      * @note This is a proprietary OpenSplice extension.
      *
      *<b>Detailed Description</b><br>
      *
      * This operation gets a property from the domain participant.
      * <p>
      * Currently, the following properties are defined:
      * <i>isolateNode</i> : The isolateNode property allows applications to isolate the federation from the 
      *                  rest of the Domain, i.e. at network level disconnect the node from the rest of the 
      *                  system. Additionally, they also need to be able to issue a request to reconnect 
      *                  their federation to the domain again after which the durability merge-policy 
      *                  that is configured needs to be applied. 
      *                  To isolate a federation, the application needs to set the isolateNode property 
      *                  value to ‘true’ and to (de)isolate the federation the same property needs to set to ‘false’. 
      *                  The default value of the isolateNode property is ‘false’.       
      *                  All data that is published after isolateNode is set to true will not be sent to the network and 
      *                  any data received from the network will be ignored. 
      *                  Be aware that data being processed by the network service at time of isolating a node may still 
      *                  be sent to the network due to asynchronous nature of network service internals.
      * </p>
      * <br>
      *
      * <i>Call</i><br>
      * This is a proprietary operation and can be called by using the operator->.
      * @code{.cpp}
      * dds::domain::DomainParticipant dp(domainId);
      * dp->get_property(...);
      * @endcode
      *
      * @param name  Indicates the name of the property to get
      * 
      * @return the value of the property
      * 
      * @throws dds::core::Error
      *                  An internal error has occurred.
      * @throws dds::core::AlreadyClosedError
      *                  The entity has already been closed.
      * @throws dds::core::OutOfResourcesError
      *                  The Data Distribution Service ran out of resources to
      *                  complete this operation.
      * @throws dds::core::UnsupportedError
      *                  The name specifies an undefined property or the operation is not supported in this version.
      */
    std::string get_property(std::string property);

#endif


    //=============================================================================
};


#endif /* OMG_TDDS_DOMAIN_DOMAIN_PARTICIPANT_HPP_ */
