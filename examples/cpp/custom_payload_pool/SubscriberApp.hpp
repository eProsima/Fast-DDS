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

#ifndef FASTDDS_EXAMPLES_CPP_CUSTOM_PAYLOAD_POOL__SUBSCRIBERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_CUSTOM_PAYLOAD_POOL__SUBSCRIBERAPP_HPP

#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "CustomPayloadPool.hpp"
#include "HelloWorld.hpp"

using namespace eprosima::fastdds::dds;
namespace eprosima {
namespace fastdds {
namespace examples {
namespace custom_payload_pool {
class SubscriberApp : public Application,  public DataReaderListener
{
public:

    SubscriberApp(
            const CLIParser::subscriber_config& config,
            const std::string& topic_name);

    virtual ~SubscriberApp();

    //! Subscription callback
    void on_data_available(
            DataReader* reader) override;

    //! Subscriber matched method
    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override;

    //!Run the subscriber until all samples have been received.
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    HelloWorld hello_;

    std::shared_ptr<CustomPayloadPool> payload_pool_;

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    Topic* topic_;

    DataReader* reader_;

    TypeSupport type_;

    uint16_t samples_;

    uint16_t received_samples_;

    //! Member used for control flow purposes
    std::atomic<bool> stop_;

    //! Protects terminate condition variable
    mutable std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    std::condition_variable terminate_cv_;
};

} // namespace custom_payload_pool
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_CUSTOM_PAYLOAD_POOL__SUBSCRIBERAPP_HPP
