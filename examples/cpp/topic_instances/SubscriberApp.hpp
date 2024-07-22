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
 * @file SubscriberApp.h
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__SUBSCRIBERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__SUBSCRIBERAPP_HPP

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "ShapeTypePubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace topic_instances {

class SubscriberApp : public Application,  public DataReaderListener
{
public:

    SubscriberApp(
            const CLIParser::subscriber_config& config);

    ~SubscriberApp();

    //! Subscription callback
    void on_data_available(
            DataReader* reader) override;

    //! Subscriber matched method
    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override;

    //! Run the subscriber
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    //! Check if all instances have received all samples
    bool instances_received_all_samples();

    //! Check if all instances have been disposed
    bool instances_disposed();

    ShapeType shape_;

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    Topic* topic_;

    DataReader* reader_;

    TypeSupport type_;

    std::condition_variable cv_;

    std::mutex mutex_;

    uint32_t samples_;

    std::atomic<bool> stop_;

    std::atomic<bool> stop_receiving_samples_;

    std::map<InstanceHandle_t, uint32_t> samples_per_instance_;

    std::vector<InstanceHandle_t> disposed_instances_;
};

} // namespace topic_instances
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__SUBSCRIBERAPP_HPP
