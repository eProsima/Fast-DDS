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

#ifndef OMG_DDS_SUB_SUBSCRIBER_LISTENER_HPP_
#define OMG_DDS_SUB_SUBSCRIBER_LISTENER_HPP_

// TODO Remove when PSM DDS Listeners are ready to be used.
#include <fastdds/dds/subscriber/SubscriberListener.hpp>

// TODO uncomment when PSM DDS Listeners are ready to be used.
//#include <dds/sub/AnyDataReaderListener.hpp>

namespace dds {
namespace sub {

class SubscriberListener;
class NoOpSubscriberListener;

/**
 * @brief
 * Subscriber events Listener
 *
 * Since a Subscriber is an Entity, it has the ability to have a Listener
 * associated with it. In this case, the associated Listener should be of type
 * SubscriberListener. This interface must be implemented by the
 * application. A user-defined class must be provided by the application which must
 * extend from the SubscriberListener class.
 *
 * <b><i>
 * All operations for this interface must be implemented in the user-defined class, it is
 * up to the application whether an operation is empty or contains some functionality.
 * </i></b>
 *
 * The SubscriberListener provides a generic mechanism (actually a
 * callback function) for the Data Distribution Service to notify the application of
 * relevant asynchronous status change events, such as a missed deadline, violation of
 * a QosPolicy setting, etc. The SubscriberListener is related to
 * changes in communication status StatusConditions.
 *
 * @code{.cpp}
 * // Application example listener
 * class ExampleListener :
 *                public virtual dds::pub::SubscriberListener
 * {
 * public:
 *     virtual void on_new_data_message (
 *         dds::pub::AnyDataReader& reader,
 *         const dds::core::status::NewDataMessageStatus& status)
 *     {
 *         std::cout << "on_new_data_message" << std::endl;
 *     }
 *
 *     virtual void on_subscription_matched (
 *         dds::pub::AnyDataReader& reader,
 *         const dds::core::status::SubscriptionMatchedStatus& status)
 *     {
 *         std::cout << "on_subscription_matched" << std::endl;
 *     }
 *
 *     virtual void on_requested_deadline_missed (
 *         dds::pub::AnyDataReader& reader,
 *         const dds::core::status::RequestedDeadlineMissedStatus& status)
 *     {
 *         std::cout << "on_requested_deadline_missed" << std::endl;
 *     }
 *
 *     virtual void on_liveliness_changed (
 *         dds::pub::AnyDataReader& reader,
 *         const dds::core::status::LivelinessChangedStatus& status)
 *     {
 *         std::cout << "on_liveliness_changed" << std::endl;
 *     }
 * };
 *
 * // Create Publisher with the listener
 * dds::domain::DomainParticipant participant(org::opensplice::domain::default_id());
 * dds::sub::Subscriber subscriber(participant,
 *                               participant.default_Subscriber_qos(),
 *                               new ExampleListener(),
 *                               dds::core::status::StatusMask::all());
 *
 * @endcode
 *
 * @see @ref DCPS_Modules_Subscriber "Subscriber"
 * @see @ref DCPS_Modules_Infrastructure_Listener "Listener information"
 */
// TODO Uncomment when PSM listeners are implemented.
//class OMG_DDS_API SubscriberListener : public virtual AnyDataReaderListener
// TODO Remove the PSM listeners are implemented.
class SubscriberListener : public eprosima::fastdds::dds::SubscriberListener
{
public:

    /** @cond */
    virtual ~SubscriberListener()
    {
    }

    /** @endcond */
};

/**
 * @brief
 * Subscriber events Listener
 *
 * This listener is just like SubscriberListener, except
 * that the application doesn't have to implement all operations.
 *
 * @code{.cpp}
 * class ExampleListener : public virtual dds::pub::NoOpSubscriberListener
 * {
 *    // Not necessary to implement any Listener operations.
 * };
 * @endcode
 *
 * @see dds::pub::SubscriberListener
 */

// TODO Uncomment when PSM DDS listeners are ready to be used
/*
   class OMG_DDS_API NoOpSubscriberListener :
        public virtual SubscriberListener,
        public virtual NoOpAnyDataReaderListener
 */
// TODO Remove the PSM listeners are implemented.
class NoOpSubscriberListener : public virtual SubscriberListener
{
public:

    /** @cond */
    virtual ~NoOpSubscriberListener()
    {
    }

    /** @endcond */
};

} //namespace sub
} //namespace dds

#endif //OMG_DDS_SUB_SUBSCRIBER_LISTENER_HPP_
