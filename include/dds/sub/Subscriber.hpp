/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef OMG_DDS_SUB_SUBSCRIBER_HPP_
#define OMG_DDS_SUB_SUBSCRIBER_HPP_

#include <dds/sub/detail/Subscriber.hpp>

#include <dds/core/Entity.hpp>
//#include <dds/sub/qos/DataReaderQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>
#include <dds/domain/DomainParticipant.hpp>

namespace dds {
namespace sub {

class SubscriberListener;

/**
 * @brief
 * The Subscriber acts on the behalf of one or several DataReader objects
 * that belong to it.
 *
 * @see @ref DCPS_Modules_Subscriber "Subscriber"
 */
class Subscriber : public dds::core::TEntity<detail::Subscriber>
{
public:

    /**
     * Local convenience typedef for dds::pub::SubscriberListener.
     */
    typedef SubscriberListener Listener;

    OMG_DDS_REF_TYPE_PROTECTED_DC(
        Subscriber,
        dds::core::TEntity,
        detail::Subscriber)

    OMG_DDS_IMPLICIT_REF_BASE(
        Subscriber)

    /**
     * Create a new Subscriber.
     *
     * The Subscriber will be created with the QoS values specified on the last
     * successful call to @link dds::domain::DomainParticipant::default_subscriber_qos(const ::dds::pub::qos::SubscriberQos& qos)
     * dp.default_subscriber_qos(qos) @endlink or, if the call was never made, the
     * @ref anchor_dds_pub_subscriber_qos_defaults "default" values.
     *
     * @param dp the domain participant
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
//    OMG_DDS_API Subscriber(
//            const dds::domain::DomainParticipant& dp);

    /**
     * Create a new Subscriber.
     *
     * The Subscriber will be created with the given QosPolicy settings and if
     * applicable, attaches the optionally specified SubscriberListener to it.
     *
     * See @ref DCPS_Modules_Infrastructure_Listener "listener" for more information
     * about listeners and possible status propagation to other entities.
     *
     * @param dp the domain participant to create the Subscriber with.
     * @param qos a collection of QosPolicy settings for the new Subscriber. In case
     *            these settings are not self consistent, no Subscriber is created.
     * @param listener the subscriber listener
     * @param mask the mask of events notified to the listener
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws dds::core::InconsistentPolicyError
     *                  The parameter qos contains conflicting QosPolicy settings.
     */
    OMG_DDS_API Subscriber(
            const dds::domain::DomainParticipant& dp,
            const qos::SubscriberQos& qos,
            SubscriberListener* listener = NULL,
            const dds::core::status::StatusMask& mask = dds::core::status::StatusMask::none());

    /** @cond */
    virtual OMG_DDS_API ~Subscriber();
    /** @endcond */

    //==========================================================================

    /**
     * Gets the SubscriberQos setting for this instance.
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
    const qos::SubscriberQos& qos() const;


    /**
     * Sets the SubscriberQos setting for this instance.
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
            const qos::SubscriberQos& qos);

    /** @copydoc dds::pub::Subscriber::qos(const dds::pub::qos::SubscriberQos& qos) */
    Subscriber& operator <<(
            const qos::SubscriberQos& qos);

    /** @copydoc dds::pub::Subscriber::qos() */
    Subscriber& operator >>(
            qos::SubscriberQos& qos);

    /**
     * Sets the default DataWriterQos of the Subscriber.
     *
     * This operation sets the default SubscriberQos of the Subscriber which
     * is used for newly created Subscriber objects, when no QoS is provided.
     *
     * This operation checks if the DataReaderQos is self consistent. If it is not, the
     * operation has no effect and throws dds::core::InconsistentPolicyError.
     *
     * The values set by this operation are returned by dds::pub::Subscriber::default_datareader_qos().
     *
     * @param qos the default DataReaderQos
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     * @throws dds::core::UnsupportedError
     *                  One or more of the selected QosPolicy values are
     *                  currently not supported.
     * @throws dds::core::InconsistentPolicyError
     *                  The parameter qos contains conflicting QosPolicy settings,
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    //    Subscriber& default_datareader_qos(
    //            const qos::DataReaderQos& qos);

    /**
     * Gets the default DataReaderQos of the Subscriber.
     *
     * This operation gets an object with the default DataReader QosPolicy settings of
     * the Subscriber (that is the DataReaderQos) which is used for newly
     * created DataReader objects, in case no QoS was provided during the creation.
     *
     * The values retrieved by this operation match the set of values specified on the last
     * successful call to
     * dds::pub::Subscriber::default_datareader_qos(const dds::pub::qos::DataReaderQos& qos),
     * or, if the call was never made, the @ref anchor_dds_pub_datareader_qos_defaults "default" values.
     *
     * @return the default DataReaderQos
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
    //    qos::DataReaderQos default_datareader_qos() const;

    //==========================================================================

    /**
     * Register a listener with the Subscriber.
     *
     * The notifications received by the listener depend on the
     * status mask with which it was registered.
     *
     * Listener un-registration is performed by setting the listener to NULL.
     *
     * See also @ref DCPS_Modules_Infrastructure_Listener "listener information".
     *
     * @param plistener the listener
     * @param mask      the mask defining the events for which the listener
     *                  will be notified.
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
            Listener* plistener,
            const dds::core::status::StatusMask& mask);

    /**
     * Get the listener of this Subscriber.
     *
     * See also @ref DCPS_Modules_Infrastructure_Listener "listener information".
     *
     * @return the listener
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     */
    Listener* listener() const;

    /**
     * Return the DomainParticipant that owns this Subscriber.
     *
     * @return the DomainParticipant
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     */
    const dds::domain::DomainParticipant& participant() const;

    dds::domain::DomainParticipant* participant_;

};

} //namespace sub
} //namespace dds

#endif //OMG_DDS_SUB_SUBSCRIBER_HPP_
