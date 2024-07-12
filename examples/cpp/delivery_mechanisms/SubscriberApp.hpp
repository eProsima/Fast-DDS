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

#ifndef FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__SUBSCRIBERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__SUBSCRIBERAPP_HPP

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

namespace eprosima {
namespace fastdds {
namespace examples {
namespace delivery_mechanisms {

class SubscriberApp : public Application, public dds::DataReaderListener
{
public:

    SubscriberApp(
            const CLIParser::delivery_mechanisms_config& config,
            const std::string& topic_name);

    ~SubscriberApp();

    //! Subscription callback
    void on_data_available(
            dds::DataReader* reader) override;

    //! Subscriber matched method
    void on_subscription_matched(
            dds::DataReader* reader,
            const dds::SubscriptionMatchedStatus& info) override;

    //! Run the subscriber
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    dds::DomainParticipant* participant_;

    dds::Subscriber* subscriber_;

    dds::Topic* topic_;

    dds::DataReader* reader_;

    dds::TypeSupport type_;

    std::condition_variable cv_;

    std::mutex mutex_;

    uint32_t received_samples_;

    uint32_t samples_;

    std::atomic<bool> stop_;
};

} // namespace delivery_mechanisms
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__SUBSCRIBERAPP_HPP
