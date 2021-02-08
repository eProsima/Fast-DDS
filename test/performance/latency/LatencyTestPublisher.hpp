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

#include <chrono>
#include <condition_variable>

#include <asio.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/TypeDescriptor.h>
#include "LatencyTestTypes.hpp"

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
            int forced_domain,
            LatencyDataSizes& latency_data_sizes);

    void run();

private:

    bool init_dynamic_types();

    bool init_static_types(
            uint32_t payload);

    bool create_data_endpoints();

    bool destroy_data_endpoints();

    bool test(
            uint32_t datasize);

    void analyze_times(
            uint32_t datasize);

    void print_stats(
            uint32_t data_index,
            TimeStats& TS);

    void export_raw_data(
            uint32_t datasize);

    void export_csv(
            const std::string& data_name,
            const std::string& str_reliable,
            const std::stringstream& data_stream);

    int32_t total_matches() const;

    /* Entities */
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::DataWriter* data_writer_;
    eprosima::fastdds::dds::DataWriter* command_writer_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::DataReader* data_reader_;
    eprosima::fastdds::dds::DataReader* command_reader_;

    /* QoS Profiles */
    eprosima::fastdds::dds::DataReaderQos dr_qos_;
    eprosima::fastdds::dds::DataWriterQos dw_qos_;

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
    int command_msg_count_;
    int data_msg_count_;
    unsigned int received_count_;
    int test_status_;

    /* Files */
    constexpr static uint32_t MINIMUM_INDEX = 0;
    constexpr static uint32_t AVERAGE_INDEX = 1;
    constexpr static uint32_t DATA_BASE_INDEX = 2;
    std::vector<std::shared_ptr<std::stringstream>> output_files_;
    std::string xml_config_file_;
    std::string raw_data_file_;
    std::string export_prefix_;

    /* Test configuration and Flags */
    bool export_csv_;
    bool reliable_;
    bool dynamic_types_ = false;
    int forced_domain_;
    int subscribers_;
    unsigned int samples_;
    bool hostname_ = false;
    uint32_t pid_ = 0;

    /* Topics */
    eprosima::fastdds::dds::Topic* latency_data_sub_topic_;
    eprosima::fastdds::dds::Topic* latency_data_pub_topic_;
    eprosima::fastdds::dds::Topic* latency_command_sub_topic_;
    eprosima::fastdds::dds::Topic* latency_command_pub_topic_;

    /* Static Types */
    LatencyType* latency_type_in_;
    LatencyType* latency_type_out_;
    eprosima::fastdds::dds::TypeSupport latency_data_type_;
    eprosima::fastdds::dds::TypeSupport latency_command_type_;

    /* Dynamic Types */
    eprosima::fastrtps::types::DynamicData* dynamic_data_type_in_;
    eprosima::fastrtps::types::DynamicData* dynamic_data_type_out_;
    eprosima::fastdds::dds::TypeSupport dynamic_pub_sub_type_;

    std::vector<uint32_t> data_size_pub_;

    /* Data Listeners */
    class LatencyDataWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        LatencyDataWriterListener(
                LatencyTestPublisher* latency_publisher)
            : latency_publisher_(latency_publisher)
            , matched_(0)
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        LatencyTestPublisher* latency_publisher_;
        int matched_;
    }
    data_writer_listener_;

    class LatencyDataReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        LatencyDataReaderListener(
                LatencyTestPublisher* latency_publisher)
            : latency_publisher_(latency_publisher)
            , matched_(0)
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        LatencyTestPublisher* latency_publisher_;
        int matched_;
    }
    data_reader_listener_;

    /* Command Listeners */
    class ComandWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        ComandWriterListener(
                LatencyTestPublisher* latency_publisher)
            : latency_publisher_(latency_publisher)
            , matched_(0)
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        LatencyTestPublisher* latency_publisher_;
        int matched_;
    }
    command_writer_listener_;

    class CommandReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        CommandReaderListener(
                LatencyTestPublisher* latency_publisher)
            : latency_publisher_(latency_publisher)
            , matched_(0)
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        LatencyTestPublisher* latency_publisher_;
        int matched_;
    }
    command_reader_listener_;
};

#endif /* LATENCYPUBLISHER_H_ */
