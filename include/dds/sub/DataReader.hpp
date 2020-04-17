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

#ifndef OMG_DDS_SUB_DATAREADER_HPP_
#define OMG_DDS_SUB_DATAREADER_HPP_

#include <dds/sub/detail/DataReader.hpp>

#include <dds/core/Entity.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>
#include <dds/sub/Subscriber.hpp>

namespace dds {
namespace sub {

class DataReaderListener;

//TODO: [ILG] Decide if we need to templatize the DataReader and derive from a AnyDataReader

/**
 * @brief
 * A DataReader allows the application to declare the data it wishes to receive.
 *
 * @see @ref DCPS_Modules_Datareader "Datareader"
 */
class DataReader : public dds::core::TEntity<detail::DataReader>
{
public:

    /**
     * Local convenience typedef for dds::pub::DataReaderListener.
     */
    typedef DataReaderListener Listener;

    OMG_DDS_REF_TYPE_PROTECTED_DC(
        DataReader,
        dds::core::TEntity,
        detail::DataReader)

    OMG_DDS_IMPLICIT_REF_BASE(
        DataReader)

    /**
     * Create a new DataReader.
     *
     * The DataReader will be created with the QoS values specified on the last
     * successful call to @link dds::sub::Subscriber::default_DataReader_qos(const ::dds::sub::qos::DataReaderQos& qos)
     * sub.default_DataReader_qos(qos) @endlink or, if the call was never made, the
     * @ref anchor_dds_sub_DataReader_qos_defaults "default" values.
     *
     * @param sub the subscriber
     * @param topic the topic the DataReader will be listening.
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    OMG_DDS_API DataReader(
            const dds::sub::Subscriber& sub,
            const dds::topic::Topic& topic);

    /**
     * Create a new DataReader.
     *
     * The DataReader will be created with the given QosPolicy settings and if
     * applicable, attaches the optionally specified DataReaderListener to it.
     *
     * See @ref DCPS_Modules_Infrastructure_Listener "listener" for more information
     * about listeners and possible status propagation to other entities.
     *
     * @param sub the subscriber to create the DataReader with.
     * @param topic the topic the DataReader will be listening.
     * @param qos a collection of QosPolicy settings for the new DataReader. In case
     *            these settings are not self consistent, no DataReader is created.
     * @param listener the DataReader listener
     * @param mask the mask of events notified to the listener
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws dds::core::InconsistentPolicyError
     *                  The parameter qos contains conflicting QosPolicy settings.
     */
    OMG_DDS_API DataReader(
            const dds::sub::Subscriber& sub,
            const dds::topic::Topic& topic,
            const qos::DataReaderQos& qos,
            DataReaderListener* listener = NULL,
            const dds::core::status::StatusMask& mask = dds::core::status::StatusMask::none());

    /** @cond */
    virtual OMG_DDS_API ~DataReader();
    /** @endcond */

    //==========================================================================

    /**
     * Gets the DataReaderQos setting for this instance.
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
    OMG_DDS_API const qos::DataReaderQos& qos() const;


    /**
     * Sets the DataReaderQos setting for this instance.
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
    OMG_DDS_API void qos(
            const qos::DataReaderQos& qos);

    /** @copydoc dds::pub::DataReader::qos(const dds::pub::qos::DataReaderQos& qos) */
    OMG_DDS_API DataReader& operator <<(
            const qos::DataReaderQos& qos);

    /** @copydoc dds::pub::DataReader::qos() */
    OMG_DDS_API DataReader& operator >>(
            qos::DataReaderQos& qos);

    //==========================================================================

    /**
     * Register a listener with the DataReader.
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
    OMG_DDS_API void listener(
            Listener* plistener,
            const dds::core::status::StatusMask& mask);

    /**
     * Get the listener of this DataReader.
     *
     * See also @ref DCPS_Modules_Infrastructure_Listener "listener information".
     *
     * @return the listener
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     */
    OMG_DDS_API Listener* listener() const;

    /**
     * Return the Subscriber that owns this DataReader.
     *
     * @return the Subscriber
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::NullReferenceError
     *                  The entity was not properly created and references to dds::core::null.
     * @throws dds::core::AlreadyClosedError
     *                  The entity has already been closed.
     */
    OMG_DDS_API const dds::sub::Subscriber& subscriber() const;

    dds::sub::Subscriber* subscriber_;

};

} //namespace sub
} //namespace dds

#endif //OMG_DDS_SUB_DATAREADER_HPP_
