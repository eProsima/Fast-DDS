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

#include <chrono>
#include <condition_variable>
#include <map>
#include <string>
#include <vector>

#include <asio.hpp>

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>

#include "../optionarg.hpp"
#include "ThroughputTypes.hpp"

class ThroughputPublisher
{
public:

    ThroughputPublisher();

    bool init(
            bool reliable,
            uint32_t pid,
            bool hostname,
            const std::string& export_csv,
            const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
            const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
            const std::string& xml_config_file,
            const std::string& demands_file,
            const std::string& recoveries_file,
            bool dynamic_types,
            Arg::EnablerValue data_sharing,
            bool data_loans,
            Arg::EnablerValue shared_memory,
            int forced_domain);

    ~ThroughputPublisher();

    bool ready();

    void run(
            uint32_t test_time,
            uint32_t recovery_time_ms,
            int demand,
            uint32_t msg_size,
            uint32_t subscribers);

private:

    bool init_dynamic_types();

    bool init_static_types(
            uint32_t payload);

    bool create_data_endpoints(
            const eprosima::fastdds::dds::DataWriterQos& dw_qos);

    bool destroy_data_endpoints();

    bool test(
            uint32_t test_time,
            uint32_t recovery_time_ms,
            uint32_t demand,
            uint32_t size);

    bool load_demands_payload();

    bool load_recoveries();

    int total_matches() const;

    // Entities
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    eprosima::fastdds::dds::DataWriter* data_writer_ = nullptr;
    eprosima::fastdds::dds::DataWriter* command_writer_ = nullptr;
    eprosima::fastdds::dds::DataReader* command_reader_ = nullptr;

    // Time
    std::chrono::steady_clock::time_point t_start_;
    std::chrono::steady_clock::time_point t_end_;
    std::chrono::duration<double, std::micro> t_overhead_;

    // Test synchronization
    std::mutex mutex_;
    std::condition_variable command_discovery_cv_;
    std::condition_variable data_discovery_cv_;

    // Topics
    eprosima::fastdds::dds::Topic* data_pub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* command_pub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* command_sub_topic_ = nullptr;

    // Data and Commands
    eprosima::fastdds::dds::TypeSupport throughput_command_type_;
    // Static Data
    ThroughputType* throughput_data_ = nullptr;
    eprosima::fastdds::dds::TypeSupport throughput_data_type_;
    // Dynamic Data
    eprosima::fastdds::dds::DynamicData::_ref_type* dynamic_data_ {nullptr};
    eprosima::fastdds::dds::TypeSupport dynamic_pub_sub_type_;
    // QoS Profiles
    eprosima::fastdds::dds::DataWriterQos dw_qos_;

    // Results
    std::vector<TroughputResults> results_;

    // Flags
    bool dynamic_types_ = false;
    Arg::EnablerValue data_sharing_ = Arg::EnablerValue::NO_SET;
    bool data_loans_ = false;
    Arg::EnablerValue shared_memory_ = Arg::EnablerValue::NO_SET;
    bool ready_ = true;
    bool reliable_ = false;
    bool hostname_ = false;
    uint32_t pid_ = 0;

    // Test configuration
    int forced_domain_ = 0;
    uint32_t payload_ = 0;
    std::map<uint32_t, std::vector<uint32_t>> demand_payload_;
    std::vector<uint32_t> recovery_times_;

    // Files
    std::string demands_file_;
    std::string export_csv_;
    std::string xml_config_file_;
    std::string recoveries_file_;

    uint32_t subscribers_ = 1;

    // Data listener
    class DataWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
        ThroughputPublisher& throughput_publisher_;
        std::atomic_int matched_;

    public:

        DataWriterListener(
                ThroughputPublisher& throughput_publisher)
            : throughput_publisher_(throughput_publisher)
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

    // Command listeners
    class CommandWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
        ThroughputPublisher& throughput_publisher_;
        std::atomic_int matched_;

    public:

        CommandWriterListener(
                ThroughputPublisher& throughput_publisher)
            : throughput_publisher_(throughput_publisher)
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
        ThroughputPublisher& throughput_publisher_;
        std::atomic_int matched_;

    public:

        CommandReaderListener(
                ThroughputPublisher& throughput_publisher)
            : throughput_publisher_(throughput_publisher)
            , matched_(0)
        {
        }

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
#endif /* THROUGHPUTPUBLISHER_H_ */
