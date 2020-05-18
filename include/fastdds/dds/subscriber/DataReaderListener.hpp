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
 * @file DataReaderListener.hpp
 */

#ifndef _FASTRTPS_DATAREADERLISTENER_HPP_
#define _FASTRTPS_DATAREADERLISTENER_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastrtps/qos/LivelinessChangedStatus.h>
#include <fastrtps/qos/SampleRejectedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DataReader;

/**
 * Class DataReaderListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup FASTDDS_MODULE
 */
class DataReaderListener
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI DataReaderListener()
    {
    }

    /**
     * @brief Destructor
     */
    RTPS_DllAPI virtual ~DataReaderListener()
    {
    }

    /**
     * Virtual function to be implemented by the user containing the actions to be performed when a new  Data Message is received.
     * @param reader DataReader
     */
    RTPS_DllAPI virtual void on_data_available(
            DataReader* reader)
    {
        (void)reader;
    }

    /**
     * Virtual method to be called when the subscriber is matched with a new Writer (or unmatched); i.e., when a writer publishing in the same topic is discovered.
     * @param reader DataReader
     * @param info The subscription matched status
     */
    RTPS_DllAPI virtual void on_subscription_matched(
            DataReader* reader,
            const fastdds::dds::SubscriptionMatchedStatus& info)
    {
        (void)reader;
        (void)info;
    }

    /**
     * Virtual method to be called when a topic misses the deadline period
     * @param reader DataReader
     * @param status The requested deadline missed status
     */
    RTPS_DllAPI virtual void on_requested_deadline_missed(
            DataReader* reader,
            const fastrtps::RequestedDeadlineMissedStatus& status)
    {
        (void)reader;
        (void)status;
    }

    /**
     * @brief Method called when the liveliness status associated to a subscriber changes
     * @param reader The DataReader
     * @param status The liveliness changed status
     */
    RTPS_DllAPI virtual void on_liveliness_changed(
            DataReader* reader,
            const fastrtps::LivelinessChangedStatus& status)
    {
        (void)reader;
        (void)status;
    }

    /**
     * @brief Method called when a sample was rejected.
     * @param reader The DataReader
     * @param status The rejected status
     */
    RTPS_DllAPI virtual void on_sample_rejected(
            DataReader* reader,
            const fastrtps::SampleRejectedStatus& status)
    {
        (void)reader;
        (void)status;
    }

    /**
     * @brief Method called an incompatible QoS was requested.
     * @param reader The DataReader
     * @param status The requested incompatible QoS status
     */
    RTPS_DllAPI virtual void on_requested_incompatible_qos(
            DataReader* reader,
            const RequestedIncompatibleQosStatus& status)
    {
        (void)reader;
        (void)status;
    }

    /**
     * @brief Method called when a sample was lost.
     * @param reader The DataReader
     * @param status The sample lost status
     */
    RTPS_DllAPI virtual void on_sample_lost(
            DataReader* reader,
            const SampleLostStatus& status)
    {
        (void)reader;
        (void)status;
    }

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTRTPS_DATAREADERLISTENER_HPP_ */
