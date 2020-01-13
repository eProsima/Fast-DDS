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

#ifndef _FASTDDS_SUBLISTENER_HPP_
#define _FASTDDS_SUBLISTENER_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class Subscriber;

/**
 * Class SubscriberListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup FASTDDS_MODULE
 */
class RTPS_DllAPI SubscriberListener
{
public:

    SubscriberListener(){}

    virtual ~SubscriberListener(){}

    /**
     * Virtual function to be implemented by the user containing the actions to be performed when a new
     * Data Message is received.
     * @param sub Subscriber
     */
    virtual void on_new_data_message(
            Subscriber* sub)
    {
        (void)sub;
    }

    /**
     * Virtual method to be called when the subscriber is matched with a new Writer (or unmatched);
     * i.e., when a writer publishing in the same topic is discovered.
     * @param sub Subscriber
     * @param info Matching information
     */
    virtual void on_subscription_matched(
            Subscriber* sub,
            const SubscriptionMatchedStatus& info)
    {
        (void)sub;
        (void)info;
    }

    /**
     * Virtual method to be called when a topic misses the deadline period
     * @param sub Subscriber
     * @param status The requested deadline missed status
     */
    virtual void on_requested_deadline_missed(
            Subscriber* sub,
            const RequestedDeadlineMissedStatus& status)
    {
        (void)sub;
        (void)status;
    }

    /**
     * Virtual method to be called when the requested qos is icompatible with the one offered
     * @param sub Subscriber
     * @param status The requested incompatible qos status
     */
    virtual void on_requested_incompatible_qos_status(
            Subscriber* sub,
            const RequestedIncompatibleQosStatus& status)
    {
        (void)sub;
        (void)status;
    }

    /**
     * @brief Method called when the liveliness status associated to a subscriber changes
     * @param sub The subscriber
     * @param status The liveliness changed status
     */
    virtual void on_liveliness_changed(
            Subscriber* sub,
            const LivelinessChangedStatus& status)
    {
        (void)sub;
        (void)status;
    }

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_SUBLISTENER_HPP_ */
