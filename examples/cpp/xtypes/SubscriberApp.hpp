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

#ifndef FASTDDS_EXAMPLES_CPP_XTYPES__SUBSCRIBERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_XTYPES__SUBSCRIBERAPP_HPP

#include <condition_variable>
#include <mutex>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include "CLIParser.hpp"
#include "Application.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace xtypes {

class SubscriberApp : public Application, public DomainParticipantListener
{
public:

    SubscriberApp(
            const CLIParser::config& config,
            const std::string& topic_name);

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
            eprosima::fastdds::rtps::WriterDiscoveryStatus reason,
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& info,
            bool& should_be_ignored) override;

    //! Run subscriber
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    //! Function to initialize the entities once the type is discovered
    void initialize_entities();

    DynamicData::_ref_type hello_;

    ::xtypes::TypeObject remote_type_object_;

    ::xtypes::TypeInformation remote_type_information_;

    DynamicType::_ref_type remote_type_;

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    const std::string topic_name_;

    Topic* topic_;

    DataReader* reader_;

    uint16_t samples_;

    uint16_t received_samples_;

    std::string type_discovered_;

    std::atomic<bool> stop_;

    mutable std::mutex mtx_;

    std::condition_variable cv_;

};

} // namespace xtypes
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_XTYPES__SUBSCRIBERAPP_HPP
