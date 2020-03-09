/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef OMG_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_
#define OMG_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_

#include <string>

#include <dds/core/detail/conformance.hpp>
#include <dds/core/refmacros.hpp>
#include <dds/core/types.hpp>
#include <dds/core/Time.hpp>
#include <dds/core/Entity.hpp>
#include <dds/domain/qos/DomainParticipantQos.hpp>

#include <dds/topic/qos/TopicQos.hpp>

#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>

#include <dds/domain/detail/DomainParticipant.hpp>

namespace dds {
namespace domain {

class DomainParticipantListener;

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
class DomainParticipant : public ::dds::core::TEntity<detail::DomainParticipant>
{
public:

    /**
     * Local representation of the dds::domain::DomainParticipantListener
     */
    using Listener = DomainParticipantListener;

public:

    OMG_DDS_REF_TYPE_PROTECTED_DC(
            DomainParticipant,
            dds::core::TEntity,
            detail::DomainParticipant)
    OMG_DDS_EXPLICIT_REF_BASE_DECL(
            DomainParticipant,
            dds::core::Entity)

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
    OMG_DDS_API DomainParticipant(
            uint32_t id);

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
    DomainParticipant(
            uint32_t id,
            const dds::domain::qos::DomainParticipantQos& qos,
            dds::domain::DomainParticipantListener* listener = NULL,
            const dds::core::status::StatusMask& event_mask = dds::core::status::StatusMask::none());

    /** @cond */
    virtual OMG_DDS_API ~DomainParticipant();
    /** @endcond */

    void OMG_DDS_API delete_participant();

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
    void listener(
            Listener* listener,
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
    const qos::DomainParticipantQos& qos() const;

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
    void qos(
            const qos::DomainParticipantQos& qos);

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
    bool contains_entity(
            const ::dds::core::InstanceHandle& handle);

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
    DomainParticipant& operator <<(
            const qos::DomainParticipantQos& qos);

    /** @copydoc dds::domain::DomainParticipant::qos() */
    const DomainParticipant& operator >>(
            qos::DomainParticipantQos& qos) const;

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
    static qos::DomainParticipantQos default_participant_qos();

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
    static void default_participant_qos(
            const ::dds::domain::qos::DomainParticipantQos& qos);

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
    DomainParticipant& default_publisher_qos(
            const ::dds::pub::qos::PublisherQos& qos);

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
    DomainParticipant& default_subscriber_qos(
            const ::dds::sub::qos::SubscriberQos& qos);

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
    dds::topic::qos::TopicQos OMG_DDS_API default_topic_qos() const;

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
    DomainParticipant& default_topic_qos(
            const dds::topic::qos::TopicQos& qos);

    //=============================================================================
};

} //namespace domain
} //namespace dds

#endif //OMG_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_
