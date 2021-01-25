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
 * @file LatencyTestSubscriber.h
 *
 */

#ifndef LATENCYTESTSUBSCRIBER_H_
#define LATENCYTESTSUBSCRIBER_H_

#include <asio.hpp>
#include <condition_variable>
#include "LatencyTestTypes.hpp"
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicPubSubType.h>

#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

class LatencyTestSubscriber
{
public:

    LatencyTestSubscriber();

    virtual ~LatencyTestSubscriber();

    bool init(
            bool echo,
            int samples,
            bool reliable,
            uint32_t pid,
            bool hostname,
            const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
            const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
            const std::string& xml_config_file,
            bool dynamic_data,
            int forced_domain,
            LatencyDataSizes& latency_data_sizes);

    void run();

    bool test(
            uint32_t datasize);

    int32_t total_matches() const;

    /* Entities */
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::DataWriter* data_writer_;
    eprosima::fastdds::dds::DataWriter* command_writer_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::DataReader* data_reader_;
    eprosima::fastdds::dds::DataReader* command_reader_;

    /* Data */
    int received_;

    /* Test synchronization */
    std::mutex mutex_;
    std::condition_variable discovery_cv_;
    std::condition_variable command_msg_cv_;
    int command_msg_count_;
    int test_status_;

    /* Files */
    std::string xml_config_file_;

    /* Test configuration and Flags */
    bool echo_;
    int samples_;
    bool dynamic_data_ = false;
    int forced_domain_;

    /* Topics */
    eprosima::fastdds::dds::Topic* latency_data_sub_topic_;
    eprosima::fastdds::dds::Topic* latency_data_pub_topic_;
    eprosima::fastdds::dds::Topic* latency_command_sub_topic_;
    eprosima::fastdds::dds::Topic* latency_command_pub_topic_;

    /* Static Types */
    LatencyType* latency_type_;
    eprosima::fastdds::dds::TypeSupport latency_data_type_;
    eprosima::fastdds::dds::TypeSupport latency_command_type_;

    /* Dynamic Types */
    eprosima::fastrtps::types::DynamicData* dynamic_data_type_;
    eprosima::fastdds::dds::TypeSupport dynamic_pub_sub_type_;

    std::vector<uint32_t> data_size_sub_;

    /* Data Listeners */
    class LatencyDataWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        LatencyDataWriterListener(
                LatencyTestSubscriber* latency_subscriber)
            : latency_subscriber_(latency_subscriber)
            , matched_(0)
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        LatencyTestSubscriber* latency_subscriber_;
        int matched_;
    }
    data_writer_listener_;

    class LatencyDataReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        LatencyDataReaderListener(
                LatencyTestSubscriber* latency_subscriber)
            : latency_subscriber_(latency_subscriber)
            , matched_(0)
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        LatencyTestSubscriber* latency_subscriber_;
        int matched_;
    }
    data_reader_listener_;

    /* Command Listeners */
    class ComandWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        ComandWriterListener(
                LatencyTestSubscriber* latency_subscriber)
            : latency_subscriber_(latency_subscriber)
            , matched_(0)
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        LatencyTestSubscriber* latency_subscriber_;
        int matched_;
    }
    command_writer_listener_;

    class CommandReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        CommandReaderListener(
                LatencyTestSubscriber* latency_subscriber)
            : latency_subscriber_(latency_subscriber)
            , matched_(0)
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        LatencyTestSubscriber* latency_subscriber_;
        int matched_;
    }
    command_reader_listener_;
};

#endif /* LATENCYTESTSUBSCRIBER_H_ */
