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

#ifndef _FASTDDS_CONFIGURATION_SUBSCRIBER_APP_HPP_
#define _FASTDDS_CONFIGURATION_SUBSCRIBER_APP_HPP_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "ConfigurationPubSubTypes.h"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace configuration {

class SubscriberApp : public Application,  public DataReaderListener
{
public:

    SubscriberApp(
            const CLIParser::subscriber_config& config);

    virtual ~SubscriberApp();

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

    Configuration configuration_;

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    Topic* topic_;

    DataReader* reader_;

    TypeSupport type_;

    static std::condition_variable cv_;

    static std::mutex mutex_;

    uint32_t received_samples_;

    uint32_t samples_;

    static std::atomic<bool> stop_;
};

} // namespace configuration
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_CONFIGURATION_SUBSCRIBER_APP_HPP_ */
