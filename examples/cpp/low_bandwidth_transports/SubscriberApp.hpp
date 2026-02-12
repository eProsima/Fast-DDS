// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SubscriberApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__SUBSCRIBERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__SUBSCRIBERAPP_HPP

#include <atomic>
#include <condition_variable>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace low_bandwidth {

class SubscriberApp : public Application, public DataReaderListener
{
public:

    //! Constructor
    SubscriberApp(
            const CLIParser::subscriber_config& config,
            const std::string& topic_name);

    //! Destructor
    ~SubscriberApp();

    //! Subscription callback
    void on_data_available(
            DataReader* reader) override;

    //! Subscriber matched method
    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override;

    //! Run subscriber
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    HelloWorld hello_;

    DomainParticipant* participant_ {nullptr};

    Subscriber* subscriber_ {nullptr};

    Topic* topic_ {nullptr};

    DataReader* reader_ {nullptr};

    TypeSupport type_ {new HelloWorldPubSubType()};

    uint16_t samples_ {0};

    uint16_t received_samples_ {0};

    std::atomic<bool> stop_ {false};

    mutable std::mutex terminate_cv_mtx_;

    std::condition_variable terminate_cv_;
};

} // namespace low_bandwidth
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_LOWBANDWIDTHTRANSPORTS__SUBSCRIBERAPP_HPP
