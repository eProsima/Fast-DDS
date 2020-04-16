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

#ifndef OMG_DDS_SUB_DATAREADER_LISTENER_HPP_
#define OMG_DDS_SUB_DATAREADER_LISTENER_HPP_

// TODO Remove when PSM DDS Listeners are ready to be used.
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

// TODO uncomment when PSM DDS Listeners are ready to be used.
//#include <dds/sub/AnyDataReaderListener.hpp>

namespace dds {
namespace sub {

class SubscriberListener;
class NoOpSubscriberListener;

/**
 * @brief
 * DataReader events Listener
 *
 * Since a DataReader is an Entity, it has the ability to have a Listener
 * associated with it. In this case, the associated Listener should be of type
 * DataReaderListener. This interface must be implemented by the
 * application. A user-defined class must be provided by the application which must
 * extend from the DataReaderListener class.
 *
 * <b><i>
 * All operations for this interface must be implemented in the user-defined class, it is
 * up to the application whether an operation is empty or contains some functionality.
 * </i></b>
 *
 * The DataReaderListener provides a generic mechanism (actually a
 * callback function) for the Data Distribution Service to notify the application of
 * relevant asynchronous status change events, such as a missed deadline, violation of
 * a QosPolicy setting, etc. The DataReaderListener is related to
 * changes in communication status StatusConditions.
 */
// TODO Uncomment when PSM listeners are implemented.
//class OMG_DDS_API DataReaderListener : public virtual AnyDataReaderListener
// TODO Remove the PSM listeners are implemented.
class DataReaderListener : public eprosima::fastdds::dds::DataReaderListener
{
public:

    /** @cond */
    virtual ~DataReaderListener()
    {
    }

    /** @endcond */
};

/**
 * @brief
 * DataReader events Listener
 *
 * This listener is just like DataReaderListener, except
 * that the application doesn't have to implement all operations.
 *
 * @code{.cpp}
 * class ExampleListener : public virtual dds::sub::NoOpDataReaderListener
 * {
 *    // Not necessary to implement any Listener operations.
 * };
 * @endcode
 *
 * @see dds::pub::DataReaderListener
 */

// TODO Uncomment when PSM DDS listeners are ready to be used
/*
   class OMG_DDS_API NoOpDataReaderListener :
        public virtual DataReaderListener,
        public virtual NoOpAnyDataReaderListener
 */
// TODO Remove the PSM listeners are implemented.
class NoOpDataReaderListener : public virtual DataReaderListener
{
public:

    /** @cond */
    virtual ~NoOpDataReaderListener()
    {
    }

    /** @endcond */
};

} //namespace sub
} //namespace dds

#endif //OMG_DDS_SUB_DATAREADER_LISTENER_HPP_
