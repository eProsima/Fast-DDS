/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef OMG_DDS_PUB_ANY_DATA_WRITER_LISTENER_HPP_
#define OMG_DDS_PUB_ANY_DATA_WRITER_LISTENER_HPP_

#include <dds/pub/AnyDataWriter.hpp>
#include <dds/core/status/Status.hpp>

namespace dds {
namespace pub {

/**
 * @brief
 * AnyDataWriter events Listener
 *
 * Because Publisher and DomainParticipant do not have knowledge of data types,
 * they have to use non-data-type-listeners. In other words Any* listeners.
 *
 * <b><i>
 * All operations for this interface must be implemented in the user-defined class, it is
 * up to the application whether an operation is empty or contains some functionality.
 *
 * This class is used as a base for other listeners and is not used on its own.
 * </i></b>
 *
 * @see dds::pub::PublisherListener
 * @see dds::domain::DomainParticipantListener
 */
class AnyDataWriterListener
{
public:

    /** @cond */
    virtual ~AnyDataWriterListener()
    {
    }

    /** @endcond */

public:

    /** @copydoc dds::pub::DataWriterListener::on_offered_deadline_missed() */
    virtual void on_offered_deadline_missed(
            AnyDataWriter& writer,
            const ::dds::core::status::OfferedDeadlineMissedStatus& status) = 0;

    /** @copydoc dds::pub::DataWriterListener::on_offered_incompatible_qos() */
    virtual void on_offered_incompatible_qos(
            AnyDataWriter& writer,
            const ::dds::core::status::OfferedIncompatibleQosStatus& status) = 0;

    /** @copydoc dds::pub::DataWriterListener::on_liveliness_lost() */
    virtual void on_liveliness_lost(
            AnyDataWriter& writer,
            const ::dds::core::status::LivelinessLostStatus& status) = 0;

    /** @copydoc dds::pub::DataWriterListener::on_publication_matched() */
    virtual void on_publication_matched(
            AnyDataWriter& writer,
            const ::dds::core::status::PublicationMatchedStatus& status) = 0;

};


/**
 * @brief
 * AnyDataWriter events Listener
 *
 * This listener is just like AnyDataWriterListener, except
 * that the application doesn't have to implement all operations.
 *
 * This class is used as a base for other listeners and is not used on its own.
 *
 * @see dds::pub::AnyDataWriterListener
 * @see dds::pub::NoOpPublisherListener
 * @see dds::domain::NoOpDomainParticipantListener
 */
class NoOpAnyDataWriterListener : public virtual AnyDataWriterListener
{
    /** @cond
     * All these functions have already been documented in the non-NoOp listener.
     * Ignore these functions for the doxygen API documentation for clarity.
     */

public:

    virtual ~NoOpAnyDataWriterListener()
    {
    }

public:

    virtual void on_offered_deadline_missed(
            AnyDataWriter& writer,
            const ::dds::core::status::OfferedDeadlineMissedStatus& status)
    {
    }

    virtual void on_offered_incompatible_qos(
            AnyDataWriter& writer,
            const ::dds::core::status::OfferedIncompatibleQosStatus& status)
    {
    }

    virtual void on_liveliness_lost(
            AnyDataWriter& writer,
            const ::dds::core::status::LivelinessLostStatus& status)
    {
    }

    virtual void on_publication_matched(
            AnyDataWriter& writer,
            const ::dds::core::status::PublicationMatchedStatus& status)
    {
    }

    /** @endcond */
};

} //namespace pub
} //namespace dds

#endif //OMG_DDS_PUB_ANY_DATA_WRITER_LISTENER_HPP_
