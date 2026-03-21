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
 * @file SubscriberListener.hpp
 */

#ifndef FASTDDS_DDS_SUBSCRIBER__SUBSCRIBERLISTENER_HPP
#define FASTDDS_DDS_SUBSCRIBER__SUBSCRIBERLISTENER_HPP

#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class Subscriber;

/**
 * Class SubscriberListener, it should be used by the end user to implement specific callbacks to certain actions.
 * It also inherits all DataReaderListener callbacks.
 *
 * @ingroup FASTDDS_MODULE
 */
class SubscriberListener : public DataReaderListener
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API SubscriberListener()
    {
    }

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~SubscriberListener()
    {
    }

    /**
     * Virtual function to be implemented by the user containing the actions to be performed when a new
     * Data Message is available on any reader.
     *
     * @param sub Subscriber
     */
    FASTDDS_EXPORTED_API virtual void on_data_on_readers(
            Subscriber* sub)
    {
        (void)sub;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_SUBSCRIBER__SUBSCRIBERLISTENER_HPP
