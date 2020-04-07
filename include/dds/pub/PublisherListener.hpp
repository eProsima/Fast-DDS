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

#ifndef OMG_DDS_PUB_PUBLISHER_LISTENER_HPP_
#define OMG_DDS_PUB_PUBLISHER_LISTENER_HPP_

// TODO Remove when PSM DDS Listeners are ready to be used.
#include <fastdds/dds/publisher/PublisherListener.hpp>

// TODO uncomment when PSM DDS Listeners are ready to be used.
//#include <dds/pub/AnyDataWriterListener.hpp>

namespace dds {
namespace pub {

class PublisherListener;
class NoOpPublisherListener;

/**
 * @brief
 * Publisher events Listener
 *
 * Since a Publisher is an Entity, it has the ability to have a Listener
 * associated with it. In this case, the associated Listener should be of type
 * PublisherListener. This interface must be implemented by the
 * application. A user-defined class must be provided by the application which must
 * extend from the PublisherListener class.
 *
 * <b><i>
 * All operations for this interface must be implemented in the user-defined class, it is
 * up to the application whether an operation is empty or contains some functionality.
 * </i></b>
 *
 * The PublisherListener provides a generic mechanism (actually a
 * callback function) for the Data Distribution Service to notify the application of
 * relevant asynchronous status change events, such as a missed deadline, violation of
 * a QosPolicy setting, etc. The PublisherListener is related to
 * changes in communication status StatusConditions.
 *
 * @code{.cpp}
 * // Application example listener
 * class ExampleListener :
 *                public virtual dds::pub::PublisherListener
 * {
 * public:
 *     virtual void on_offered_deadline_missed (
 *         dds::pub::AnyDataWriter& writer,
 *         const dds::core::status::OfferedDeadlineMissedStatus& status)
 *     {
 *         std::cout << "on_offered_deadline_missed" << std::endl;
 *     }
 *
 *     virtual void on_offered_incompatible_qos (
 *         dds::pub::AnyDataWriter& writer,
 *         const dds::core::status::OfferedIncompatibleQosStatus& status)
 *     {
 *         std::cout << "on_offered_incompatible_qos" << std::endl;
 *     }
 *
 *     virtual void on_liveliness_lost (
 *         dds::pub::AnyDataWriter& writer,
 *         const dds::core::status::LivelinessLostStatus& status)
 *     {
 *         std::cout << "on_liveliness_lost" << std::endl;
 *     }
 *
 *     virtual void on_publication_matched (
 *         dds::pub::AnyDataWriter& writer,
 *         const dds::core::status::PublicationMatchedStatus& status)
 *     {
 *         std::cout << "on_publication_matched" << std::endl;
 *     }
 * };
 *
 * // Create Publisher with the listener
 * dds::domain::DomainParticipant participant(org::opensplice::domain::default_id());
 * dds::pub::Publisher publisher(participant,
 *                               participant.default_publisher_qos(),
 *                               new ExampleListener(),
 *                               dds::core::status::StatusMask::all());
 *
 * @endcode
 *
 * @see @ref DCPS_Modules_Publisher "Publisher"
 * @see @ref DCPS_Modules_Infrastructure_Listener "Listener information"
 */
// TODO Uncomment when PSM listeners are implemented.
//class OMG_DDS_API PublisherListener : public virtual AnyDataWriterListener
// TODO Remove the PSM listeners are implemented.
class PublisherListener : public eprosima::fastdds::dds::PublisherListener
{
public:

    /** @cond */
    virtual ~PublisherListener()
    {
    }

    /** @endcond */
};

/**
 * @brief
 * Publisher events Listener
 *
 * This listener is just like PublisherListener, except
 * that the application doesn't have to implement all operations.
 *
 * @code{.cpp}
 * class ExampleListener : public virtual dds::pub::NoOpPublisherListener
 * {
 *    // Not necessary to implement any Listener operations.
 * };
 * @endcode
 *
 * @see dds::pub::PublisherListener
 */

// TODO Uncomment when PSM DDS listeners are ready to be used
/*
   class OMG_DDS_API NoOpPublisherListener :
        public virtual PublisherListener,
        public virtual NoOpAnyDataWriterListener
 */
// TODO Remove the PSM listeners are implemented.
class NoOpPublisherListener : public virtual PublisherListener
{
public:

    /** @cond */
    virtual ~NoOpPublisherListener()
    {
    }

    /** @endcond */
};

} //namespace pub
} //namespace dds

#endif //OMG_DDS_PUB_PUBLISHER_LISTENER_HPP_
