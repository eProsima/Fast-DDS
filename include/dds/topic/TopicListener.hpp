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

#ifndef OMG_DDS_SUB_TOPIC_LISTENER_HPP_
#define OMG_DDS_SUB_TOPIC_LISTENER_HPP_

// TODO Remove when PSM DDS Listeners are ready to be used.
#include <fastdds/dds/topic/TopicListener.hpp>

// TODO uncomment when PSM DDS Listeners are ready to be used.
//#include <dds/topic/AnyDataReaderListener.hpp>

namespace dds {
namespace topic {

/**
 * @brief
 * Topic events Listener
 *
 * Since a Topic is an Entity, it has the ability to have a Listener
 * associated with it. In this case, the associated Listener should be of type
 * TopicListener. This interface must be implemented by the
 * application. A user-defined class must be provided by the application which must
 * extend from the TopicListener class.
 *
 * <b><i>
 * All operations for this interface must be implemented in the user-defined class, it is
 * up to the application whether an operation is empty or contains some functionality.
 * </i></b>
 *
 * The TopicListener provides a generic mechanism (actually a
 * callback function) for the Data Distribution Service to notify the application of
 * relevant asynchronous status change events, such as a missed deadline, violation of
 * a QosPolicy setting, etc. The TopicListener is related to
 * changes in communication status StatusConditions.
 *
 * @see @ref DCPS_Modules_Topic "Topic"
 * @see @ref DCPS_Modules_Infrastructure_Listener "Listener information"
 */
// TODO Uncomment when PSM listeners are implemented.
//class OMG_DDS_API TopicListener : public virtual AnyDataReaderListener
// TODO Remove the PSM listeners are implemented.
class TopicListener : public eprosima::fastdds::dds::TopicListener
{
public:

    /** @cond */
    virtual ~TopicListener()
    {
    }

    /** @endcond */
};

/**
 * @brief
 * Topic events Listener
 *
 * This listener is just like TopicListener, except
 * that the application doesn't have to implement all operations.
 *
 * @code{.cpp}
 * class ExampleListener : public virtual dds::pub::NoOpTopicListener
 * {
 *    // Not necessary to implement any Listener operations.
 * };
 * @endcode
 *
 * @see dds::pub::TopicListener
 */

// TODO Uncomment when PSM DDS listeners are ready to be used
/*
   class OMG_DDS_API NoOpTopicListener :
        public virtual TopicListener,
        public virtual NoOpAnyDataReaderListener
 */
// TODO Remove the PSM listeners are implemented.
class NoOpTopicListener : public virtual TopicListener
{
public:

    /** @cond */
    virtual ~NoOpTopicListener()
    {
    }

    /** @endcond */
};

} //namespace topic
} //namespace dds

#endif //OMG_DDS_SUB_TOPIC_LISTENER_HPP_
