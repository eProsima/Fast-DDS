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
#include <fastrtps/fastrtps_fwd.h>
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

    bool init_dynamic_types();

    bool init_static_types(uint32_t payload);

    bool create_data_endpoints();

    bool destroy_data_endpoints();

    bool test(
            uint32_t test_time,
            uint32_t recovery_time_ms,
            uint32_t demand,
            uint32_t size);

    bool load_demands_payload();

    bool load_recoveries();

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
    std::mutex command_mutex_;
    std::mutex data_mutex_;
    std::condition_variable command_discovery_cv_;
    std::condition_variable data_discovery_cv_;
    int command_discovery_count_ = 0;
    int data_discovery_count_ = 0;

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
    eprosima::fastrtps::types::DynamicData* dynamic_data_ = nullptr;
    eprosima::fastdds::dds::TypeSupport dynamic_pub_sub_type_;
    eprosima::fastdds::dds::DataWriterQos dw_qos_;

    // Results
    std::vector<TroughputResults> results_;

    // Flags
    bool dynamic_types_ = false;
    bool ready_ = true;
    bool reliable_ = false;
    bool hostname_ = false;
    uint32_t pid_ = 0;

    // Test configuration
    int forced_domain_ = 0;
    uint32_t payload_ = 0;
    std::map<uint32_t, std::vector<uint32_t> > demand_payload_;
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

    public:

        DataWriterListener(
                ThroughputPublisher& throughput_publisher)
            : throughput_publisher_(throughput_publisher)
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;
    }
    data_writer_listener_;

    // Command listeners
    class CommandWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
        ThroughputPublisher& throughput_publisher_;

    public:

        CommandWriterListener(
                ThroughputPublisher& throughput_publisher)
            : throughput_publisher_(throughput_publisher)
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;
    }
    command_writer_listener_;

    class CommandReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        ThroughputPublisher& throughput_publisher_;

    public:

        CommandReaderListener(
                ThroughputPublisher& throughput_publisher)
            : throughput_publisher_(throughput_publisher)
        {
        }

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;
    }
    command_reader_listener_;
};
#endif /* THROUGHPUTPUBLISHER_H_ */
