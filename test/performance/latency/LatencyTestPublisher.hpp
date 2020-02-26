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
 * @file LatencyPublisher.h
 *
 */

#ifndef LATENCYPUBLISHER_H_
#define LATENCYPUBLISHER_H_

#include <asio.hpp>

#include "LatencyTestTypes.hpp"

#include <condition_variable>
#include <chrono>

#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicPubSubType.h>

class TimeStats
{
public:

    TimeStats()
        : bytes_(0)
        , received_(0)
        , minimum_(0)
        , maximum_(0)
        , percentile_50_(0)
        , percentile_90_(0)
        , percentile_99_(0)
        , percentile_9999_(0)
        , mean_(0)
        , stdev_(0)
    {
    }

    ~TimeStats()
    {
    }

    uint64_t bytes_;
    unsigned int received_;
    std::chrono::duration<double, std::micro> minimum_;
    std::chrono::duration<double, std::micro> maximum_;
    double percentile_50_;
    double percentile_90_;
    double percentile_99_;
    double percentile_9999_;
    double mean_;
    double stdev_;
};

class LatencyTestPublisher
{

public:

    LatencyTestPublisher();

    virtual ~LatencyTestPublisher();

    bool init(
            int subscribers,
            int samples,
            bool reliable,
            uint32_t pid,
            bool hostname,
            bool export_csv,
            const std::string& export_prefix,
            std::string raw_data_file,
            const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
            const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
            const std::string& xml_config_file,
            bool dynamic_data,
            int forced_domain);

    void run();

    bool test(
            uint32_t datasize);

    void analyze_times(
            uint32_t datasize);

    void print_stats(
            TimeStats& TS);

    void export_raw_data(
            uint32_t datasize);

    /* Entities */
    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Publisher* data_publisher_;
    eprosima::fastrtps::Publisher* command_publisher_;
    eprosima::fastrtps::Subscriber* data_subscriber_;
    eprosima::fastrtps::Subscriber* command_subscriber_;

    /* Times */
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point end_time_;
    std::chrono::duration<double, std::micro> overhead_time_;
    std::vector<std::chrono::duration<double, std::micro>> times_;

    /* Data */
    eprosima::fastrtps::SampleInfo_t sampleinfo_;
    std::vector<TimeStats> stats_;
    uint64_t raw_sample_count_;

    /* Test synchronization */
    std::mutex mutex_;
    std::condition_variable discovery_cv_;
    std::condition_variable command_msg_cv_;
    std::condition_variable data_msg_cv_;
    int discovery_count_;
    int command_msg_count_;
    int data_msg_count_;
    unsigned int received_count_;
    int test_status_;

    /* Files */
    std::stringstream output_file_minimum_;
    std::stringstream output_file_average_;
    std::stringstream output_file_16_;
    std::stringstream output_file_32_;
    std::stringstream output_file_64_;
    std::stringstream output_file_128_;
    std::stringstream output_file_256_;
    std::stringstream output_file_512_;
    std::stringstream output_file_1024_;
    std::stringstream output_file_2048_;
    std::stringstream output_file_4096_;
    std::stringstream output_file_8192_;
    std::stringstream output_file_16384_;
    std::stringstream output_file_64000_;
    std::stringstream output_file_131072_;
    std::string xml_config_file_;
    std::string raw_data_file_;
    std::string export_prefix_;

    /* Test configuration and Flags */
    bool export_csv_;
    bool reliable_;
    bool dynamic_data_ = false;
    int forced_domain_;
    int subscribers_;
    unsigned int samples_;

    /* Static Types */
    LatencyDataType latency_data_type_;
    LatencyType* latency_type_in_;
    LatencyType* latency_type_out_;
    TestCommandDataType latency_command_type_;

    /* Dynamic Types */
    eprosima::fastrtps::types::DynamicData* dynamic_data_type_in_;
    eprosima::fastrtps::types::DynamicData* dynamic_data_type_out_;
    eprosima::fastrtps::types::DynamicPubSubType dynamic_pub_sub_type_;
    eprosima::fastrtps::types::DynamicType_ptr dynamic_type_;

    /* Data Listeners */
    class DataPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:

        DataPubListener(
                LatencyTestPublisher* latency_publisher)
            : latency_publisher_(latency_publisher)
            , matched_(0)
        {
        }

        ~DataPubListener()
        {
        }

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        LatencyTestPublisher* latency_publisher_;
        int matched_;
    } data_pub_listener_;

    class DataSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        DataSubListener(
                LatencyTestPublisher* latency_publisher)
            : latency_publisher_(latency_publisher)
            , matched_(0)
        {
        }

        ~DataSubListener()
        {
        }

        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber* sub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        void onNewDataMessage(
                eprosima::fastrtps::Subscriber* sub);

        LatencyTestPublisher* latency_publisher_;
        int matched_;
    } data_sub_listener_;

    /* Command Listeners */
    class CommandPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:

        CommandPubListener(
                LatencyTestPublisher* latency_publisher)
            : latency_publisher_(latency_publisher)
            , matched_(0)
        {
        }

        ~CommandPubListener()
        {
        }

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        LatencyTestPublisher* latency_publisher_;
        int matched_;
    } command_pub_listener_;

    class CommandSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        CommandSubListener(
                LatencyTestPublisher* latency_publisher)
            : latency_publisher_(latency_publisher)
            , matched_(0)
        {
        }

        ~CommandSubListener()
        {
        }

        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber* sub,
                eprosima::fastrtps::rtps::MatchingInfo& info);
        void onNewDataMessage(
                eprosima::fastrtps::Subscriber* sub);

        LatencyTestPublisher* latency_publisher_;
        int matched_;
    } command_sub_listener_;
};

#endif /* LATENCYPUBLISHER_H_ */
