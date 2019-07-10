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
 * @file DataReaderListener.h
 */

#ifndef _FASTRTPS_DATAREADERLISTENER_H_
#define _FASTRTPS_DATAREADERLISTENER_H_

#include "../../fastrtps/fastrtps_dll.h"
#include "../../fastrtps/qos/DeadlineMissedStatus.h"
#include "../../fastrtps/qos/LivelinessChangedStatus.h"
#include "../../fastrtps/qos/SampleLostStatus.hpp"
#include "../../fastrtps/qos/SampleRejectedStatus.hpp"
#include "../../fastrtps/qos/IncompatibleQosStatus.hpp"

namespace eprosima {
namespace fastrtps {

namespace rtps {
class MatchingInfo;
} /* namespace rtps */
} /* namespace fastrtps */

namespace fastdds {

class DataReader;

/**
 * Class DataReaderListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup FASTRTPS_MODULE
 * @snippet fastrtps_example.cpp ex_DataReaderListener
 */
class RTPS_DllAPI DataReaderListener
{
    public:

        DataReaderListener(){}

        virtual ~DataReaderListener(){}

        /**
         * Virtual function to be implemented by the user containing the actions to be performed when a new  Data Message is received.
         * @param reader DataReader
         */
        virtual void on_data_available(
                DataReader* reader)
        {
            (void)reader;
        }

        /**
         * Virtual method to be called when the subscriber is matched with a new Writer (or unmatched); i.e., when a writer publishing in the same topic is discovered.
         * @param reader DataReader
         * @param info Matching information
         */
        virtual void on_subscription_matched(
                DataReader* reader,
                fastrtps::rtps::MatchingInfo& info)
        {
            (void)reader;
            (void)info;
        }

        /**
         * Virtual method to be called when a topic misses the deadline period
         * @param reader DataReader
         * @param status The requested deadline missed status
         */
        virtual void on_requested_deadline_missed(
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
        virtual void on_liveliness_changed(
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
        virtual void on_sample_rejected(
                DataReader* reader,
                const fastrtps::SampleRejectedStatus& status)
        {
            (void)reader;
            (void)status;
        }

        /**
         * @brief Method called an incompatible QoS was requested.
         * @param reader The DataReader
         * @param status The RequestedIncompatibleQos status
         */
        virtual void on_requested_incompatible_qos(
                DataReader* reader,
                const fastrtps::RequestedIncompatibleQosStatus& status)
        {
            (void)reader;
            (void)status;
        }

        /**
         * @brief Method called when a sample was lost.
         * @param reader The DataReader
         * @param status The SampleLost status
         */
        virtual void on_sample_rejected(
                DataReader* reader,
                const fastrtps::SampleLostStatus& status)
        {
            (void)reader;
            (void)status;
        }
};

} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTRTPS_DATAREADERLISTENER_H_ */
