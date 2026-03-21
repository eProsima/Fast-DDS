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
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include "LatencyTestTypes.hpp"

#include "../optionarg.hpp"

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
            const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
            const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
            const std::string& xml_config_file,
            bool dynamic_data,
            Arg::EnablerValue data_sharing,
            bool data_loans,
            Arg::EnablerValue shared_memory,
            int forced_domain,
            LatencyDataSizes& latency_data_sizes);

    void run();

    void destroy_user_entities();

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

    template<class Predicate>
    void wait_for_discovery(
            Predicate pred)
    {
        std::unique_lock<std::mutex> disc_lock(mutex_);
        discovery_cv_.wait(disc_lock, pred);
    }

    template<class Predicate>
    void wait_for_command(
            Predicate pred)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        command_msg_cv_.wait(lock, pred);
        command_msg_count_ = 0;
    }

    /* Entities */
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
    eprosima::fastdds::dds::DataWriter* data_writer_ = nullptr;
    eprosima::fastdds::dds::DataWriter* command_writer_ = nullptr;
    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    eprosima::fastdds::dds::DataReader* data_reader_ = nullptr;
    eprosima::fastdds::dds::DataReader* command_reader_ = nullptr;

    /* QoS Profiles */
    eprosima::fastdds::dds::DataReaderQos dr_qos_;
    eprosima::fastdds::dds::DataWriterQos dw_qos_;

    /* Times */
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point end_time_;
    std::chrono::duration<double, std::micro> overhead_time_;
    std::vector<std::chrono::duration<double, std::micro>> times_;

    /* Data */
    eprosima::fastdds::dds::SampleInfo sampleinfo_;
    std::vector<TimeStats> stats_;
    uint64_t raw_sample_count_ = 0;

    /* Test synchronization */
    std::mutex mutex_;
    std::condition_variable discovery_cv_;
    std::condition_variable command_msg_cv_;
    std::condition_variable data_msg_cv_;
    int command_msg_count_ = 0;
    int data_msg_count_ = 0;
    unsigned int received_count_ = 0;
    int test_status_ = 0;

    /* Files */
    constexpr static uint32_t MINIMUM_INDEX = 0;
    constexpr static uint32_t AVERAGE_INDEX = 1;
    constexpr static uint32_t DATA_BASE_INDEX = 2;
    std::vector<std::shared_ptr<std::stringstream>> output_files_;
    std::string xml_config_file_;
    std::string raw_data_file_;
    std::string export_prefix_;

    /* Test configuration and Flags */
    bool export_csv_ = false;
    bool reliable_ = false;
    bool dynamic_types_ = false;
    Arg::EnablerValue data_sharing_ = Arg::EnablerValue::NO_SET;
    bool data_loans_ = false;
    Arg::EnablerValue shared_memory_ = Arg::EnablerValue::NO_SET;
    int forced_domain_ = -1;
    int subscribers_ = 0;
    unsigned int samples_ = 0;
    bool hostname_ = false;
    uint32_t pid_ = 0;

    /* Topics */
    eprosima::fastdds::dds::Topic* latency_data_sub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* latency_data_pub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* latency_command_sub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* latency_command_pub_topic_ = nullptr;

    /* Static Types */
    LatencyType* latency_data_in_ = nullptr;
    LatencyType* latency_data_out_ = nullptr;
    eprosima::fastdds::dds::TypeSupport latency_data_type_;
    eprosima::fastdds::dds::TypeSupport latency_command_type_;

    /* Dynamic Types */
    eprosima::fastdds::dds::DynamicData::_ref_type* dynamic_data_in_ {nullptr};
    eprosima::fastdds::dds::DynamicData::_ref_type* dynamic_data_out_ {nullptr};
    eprosima::fastdds::dds::TypeSupport dynamic_pub_sub_type_;

    std::vector<uint32_t> data_size_pub_;

    /* Data Listeners */
    class LatencyDataWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
        LatencyTestPublisher* latency_publisher_;
        int matched_;

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

        int get_matches() const
        {
            return matched_;
        }

        void reset()
        {
            matched_ = 0;
        }

    }
    data_writer_listener_;

    class LatencyDataReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        LatencyTestPublisher* latency_publisher_;
        int matched_;

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

        int get_matches() const
        {
            return matched_;
        }

        void reset()
        {
            matched_ = 0;
        }

    }
    data_reader_listener_;

    /* Command Listeners */
    class ComandWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
        LatencyTestPublisher* latency_publisher_;
        int matched_;

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

        int get_matches() const
        {
            return matched_;
        }

        void reset()
        {
            matched_ = 0;
        }

    }
    command_writer_listener_;

    class CommandReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        LatencyTestPublisher* latency_publisher_;
        int matched_;

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

        int get_matches() const
        {
            return matched_;
        }

        void reset()
        {
            matched_ = 0;
        }

    }
    command_reader_listener_;
};

#endif /* LATENCYPUBLISHER_H_ */
