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

#ifndef FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__SUBSCRIBERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__SUBSCRIBERAPP_HPP

#include <atomic>
#include <condition_variable>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "CustomContentFilterFactory.hpp"
#include "HelloWorld.hpp"

using namespace eprosima::fastdds::dds;
namespace eprosima {
namespace fastdds {
namespace examples {
namespace content_filter {
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

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    Topic* topic_;

    DataReader* reader_;

    TypeSupport type_;

    uint16_t received_samples_;

    uint16_t samples_;

    //! DDS ContentFilteredTopic pointer
    ContentFilteredTopic* filter_topic_;

    //! Custom filter factory
    CustomContentFilterFactory filter_factory;

    std::atomic<bool> stop_;

    mutable std::mutex terminate_cv_mtx_;

    std::condition_variable terminate_cv_;
};

} // namespace content_filter
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__SUBSCRIBERAPP_HPP
