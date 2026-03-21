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

#include <condition_variable>

#include <asio.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include "LatencyTestTypes.hpp"

#include "../optionarg.hpp"

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

    bool test(
            uint32_t datasize);

private:

    bool init_dynamic_types();

    bool init_static_types(
            uint32_t payload);

    bool create_data_endpoints();

    bool destroy_data_endpoints();

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

    /* Data */
    int received_;

    /* Test synchronization */
    std::mutex mutex_;
    std::condition_variable discovery_cv_;
    std::condition_variable command_msg_cv_;
    int command_msg_count_ = 0;
    int test_status_ = 0;

    /* Files */
    std::string xml_config_file_;

    /* Test configuration and Flags */
    bool echo_ = true;
    int samples_ = 0;
    bool dynamic_types_ = false;
    Arg::EnablerValue data_sharing_ = Arg::EnablerValue::NO_SET;
    bool data_loans_ = false;
    Arg::EnablerValue shared_memory_ = Arg::EnablerValue::NO_SET;
    int forced_domain_ = -1;
    bool hostname_ = false;
    uint32_t pid_ = 0;

    /* Topics */
    eprosima::fastdds::dds::Topic* latency_data_sub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* latency_data_pub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* latency_command_sub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* latency_command_pub_topic_ = nullptr;

    /* Static Types */
    LatencyType* latency_data_ = nullptr;
    eprosima::fastdds::dds::TypeSupport latency_data_type_;
    eprosima::fastdds::dds::TypeSupport latency_command_type_;

    /* Dynamic Types */
    eprosima::fastdds::dds::DynamicData::_ref_type* dynamic_data_ {nullptr};
    eprosima::fastdds::dds::TypeSupport dynamic_pub_sub_type_;

    std::vector<uint32_t> data_size_sub_;

    /* Data Listeners */
    class LatencyDataWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
        LatencyTestSubscriber* latency_subscriber_;
        int matched_;

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
        LatencyTestSubscriber* latency_subscriber_;
        int matched_;

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
        LatencyTestSubscriber* latency_subscriber_;
        int matched_;

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
        LatencyTestSubscriber* latency_subscriber_;
        int matched_;

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

#endif /* LATENCYTESTSUBSCRIBER_H_ */
