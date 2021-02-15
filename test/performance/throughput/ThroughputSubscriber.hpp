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
 * @file ThroughputSubscriber.h
 *
 */

#ifndef THROUGHPUTSUBSCRIBER_H_
#define THROUGHPUTSUBSCRIBER_H_

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>

#include <asio.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastrtps/rtps/attributes/PropertyPolicy.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/TypeDescriptor.h>
#include "ThroughputTypes.hpp"

class ThroughputSubscriber
{
public:

    ThroughputSubscriber();

    bool init(
            bool reliable,
            uint32_t pid,
            bool hostname,
            const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
            const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
            const std::string& xml_config_file,
            bool dynamic_types,
            bool data_sharing,
            bool data_loans,
            int forced_domain);

    ~ThroughputSubscriber();

    bool ready();

    void run();

private:

    bool init_dynamic_types();

    bool init_static_types(
            uint32_t payload);

    bool create_data_endpoints();

    bool destroy_data_endpoints();

    // return value: 0 - Continuing test, 1 - End of a test, 2 - Finish application
    int process_message();

    int total_matches() const;

    // Entities
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    eprosima::fastdds::dds::DataReader* data_reader_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
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
    uint32_t command_discovery_count_ = 0;
    uint32_t data_discovery_count_ = 0;

    // Topics
    eprosima::fastdds::dds::Topic* data_sub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* command_pub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* command_sub_topic_ = nullptr;

    // Data and commands
    eprosima::fastdds::dds::TypeSupport throughput_command_type_;
    // Static Data
    ThroughputType* throughput_data_ = nullptr;
    eprosima::fastdds::dds::TypeSupport throughput_data_type_;
    // Dynamic Data
    eprosima::fastrtps::types::DynamicData* dynamic_data_ = nullptr;
    eprosima::fastdds::dds::TypeSupport dynamic_pub_sub_type_;
    // QoS Profiles
    eprosima::fastdds::dds::DataReaderQos dr_qos_;

    // Flags
    bool dynamic_types_ = false;
    bool data_sharing_ = false;
    bool data_loans_ = false;
    bool ready_ = true;
    bool reliable_ = false;
    bool hostname_ = false;
    uint32_t pid_ = 0;

    // Test configuration
    uint32_t data_size_ = 0;
    uint32_t demand_ = 0;
    int forced_domain_;

    // Files
    std::string xml_config_file_;

    // Data listener
    class DataReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        ThroughputSubscriber& throughput_subscriber_;
        uint32_t last_seq_num_ = 0;
        uint32_t lost_samples_ = 0;
        eprosima::fastdds::dds::SampleInfo info_;
        int matched_ = 0;
        bool enable_ = false;

    public:

        DataReaderListener(
                ThroughputSubscriber& throughput_subscriber)
            : throughput_subscriber_(throughput_subscriber)
        {
        }

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void reset();

        void disable();

        void save_numbers();

        int get_matches() const
        {
            return matched_;
        }

        uint32_t saved_last_seq_num_;
        uint32_t saved_lost_samples_;
    }
    data_reader_listener_;

    // Command listeners
    class CommandReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        ThroughputSubscriber& throughput_subscriber_;
        int matched_ = 0;

    public:

        CommandReaderListener(
                ThroughputSubscriber& throughput_subscriber)
            : throughput_subscriber_(throughput_subscriber)
        {
        }

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void save_numbers();

        int get_matches() const
        {
            return matched_;
        }

    }
    command_reader_listener_;

    class CommandWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
        ThroughputSubscriber& throughput_subscriber_;
        int matched_ = 0;

    public:

        CommandWriterListener(
                ThroughputSubscriber& throughput_subscriber)
            : throughput_subscriber_(throughput_subscriber)
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        int get_matches() const
        {
            return matched_;
        }

    }
    command_writer_listener_;
};
#endif /* THROUGHPUTSUBSCRIBER_H_ */
