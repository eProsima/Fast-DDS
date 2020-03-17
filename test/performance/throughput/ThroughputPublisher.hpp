// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ThroughputPublisher.h
 *
 */

#ifndef THROUGHPUTPUBLISHER_H_
#define THROUGHPUTPUBLISHER_H_

#include <asio.hpp>

#include "ThroughputTypes.hpp"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/rtps/attributes/PropertyPolicy.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicPubSubType.h>

#include <condition_variable>
#include <chrono>
#include <map>
#include <vector>
#include <string>

class ThroughputPublisher
{
public:

    ThroughputPublisher(
            bool reliable,
            uint32_t pid,
            bool hostname,
            const std::string& export_csv,
            const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
            const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
            const std::string& xml_config_file,
            const std::string& demands_file,
            const std::string& recoveries_file,
            bool dynamic_types,
            int forced_domain);

    virtual ~ThroughputPublisher();

    bool ready();

    void run(
            uint32_t test_time,
            uint32_t recovery_time_ms,
            int demand,
            int msg_size,
            uint32_t subscribers);

private:

    bool test(
            uint32_t test_time,
            uint32_t recovery_time_ms,
            uint32_t demand,
            uint32_t size);

    bool load_demands_payload();

    bool load_recoveries();

    // Entities
    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Publisher* data_publisher_;
    eprosima::fastrtps::Publisher* command_publisher_;
    eprosima::fastrtps::Subscriber* command_subscriber_;

    // Time
    std::chrono::steady_clock::time_point t_start_;
    std::chrono::steady_clock::time_point t_end_;
    std::chrono::duration<double, std::micro> t_overhead_;

    // Test synchronization
    std::mutex command_mutex_;
    std::mutex data_mutex_;
    std::condition_variable command_discovery_cv_;
    std::condition_variable data_discovery_cv_;
    int command_discovery_count_;
    int data_discovery_count_;

    // Data and Commands
    ThroughputCommandDataType throuput_command_type_;
    // Static Data
    ThroughputDataType* throughput_data_type_;
    ThroughputType* throughput_type_;
    // Dynamic Data
    eprosima::fastrtps::types::DynamicData* dynamic_data_type_;
    eprosima::fastrtps::types::DynamicPubSubType dynamic_pub_sub_type_;
    eprosima::fastrtps::types::DynamicType_ptr dynamic_type_;
    eprosima::fastrtps::PublisherAttributes pub_attrs_;

    // Results
    std::vector<TroughputResults> results_;

    // Flags
    bool dynamic_data_ = false;
    bool ready_;
    bool reliable_;

    // Test configuration
    int forced_domain_;
    uint32_t payload_;
    std::map<uint32_t,std::vector<uint32_t>> demand_payload_;
    std::vector<uint32_t> recovery_times_;

    // Files
    std::string demands_file_;
    std::string export_csv_;
    std::string xml_config_file_;
    std::string recoveries_file_;

    uint32_t subscribers_;

    // Data listener
    class DataPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:

        DataPubListener(
                ThroughputPublisher& throughput_publisher);

        virtual ~DataPubListener();

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        ThroughputPublisher& throughput_publisher_;

    private:
        DataPubListener& operator=(
                const DataPubListener&);
    } data_pub_listener_;

    // Command listeners
    class CommandPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:
        CommandPubListener(
                ThroughputPublisher& throughput_publisher);

        virtual ~CommandPubListener();

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        ThroughputPublisher& throughput_publisher_;

    private:
        CommandPubListener& operator=(
                const CommandPubListener&);
    } command_pub_listener_;

    class CommandSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        CommandSubListener(
                ThroughputPublisher& throughput_publisher);

        virtual ~CommandSubListener();

        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber* sub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        ThroughputPublisher& throughput_publisher_;

    private:
        CommandSubListener& operator=(
                const CommandSubListener&);
    } command_sub_listener_;
};
#endif /* THROUGHPUTPUBLISHER_H_ */
