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


#ifndef FASTDDS_FLOW_CONTROL_SUBSCRIBER_APP_HPP
#define FASTDDS_FLOW_CONTROL_SUBSCRIBER_APP_HPP

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "FlowControlPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace flow_control {

class SubscriberApp : public Application,  public DomainParticipantListener
{
public:

    SubscriberApp(
            const CLIParser::flow_control_config& config);

    ~SubscriberApp();

    //! Subscription callback
    void on_data_available(
            DataReader* reader) override;

    //! Subscriber matched method
    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override;

    void on_data_writer_discovery(
            DomainParticipant* participant,
            eprosima::fastdds::rtps::WriterDiscoveryStatus status,
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& info,
            bool& should_be_ignored) override;

    //! Run subscriber
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    Topic* topic_;

    DataReader* reader_;

    TypeSupport type_;

    uint16_t samples_;

    std::atomic<bool> stop_;

    mutable std::mutex terminate_cv_mtx_;

    std::condition_variable terminate_cv_;

    std::vector<eprosima::fastdds::rtps::GUID_t> slow_writer_guid;

    std::vector<eprosima::fastdds::rtps::GUID_t> fast_writer_guid;

};

} // namespace flow_control
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_FLOW_CONTROL_SUBSCRIBER_APP_HPP
