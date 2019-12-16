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
            bool large_data,
            const std::string& xml_config_file,
            bool dynamic_data,
            int forced_domain);

    void run();

    bool test(
            uint32_t datasize);

    /* Entities */
    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Publisher* data_publisher_;
    eprosima::fastrtps::Publisher* command_publisher_;
    eprosima::fastrtps::Subscriber* data_subscriber_;
    eprosima::fastrtps::Subscriber* command_subscriber_;

    /* Data */
    eprosima::fastrtps::SampleInfo_t sample_info_;
    int received_;

    /* Test synchronization */
    std::mutex mutex_;
    std::condition_variable discovery_cv_;
    std::condition_variable command_msg_cv_;
    int discovery_count_;
    int command_msg_count_;
    int test_status_;

    /* Files */
    std::string xml_config_file_;

    /* Test configuration and Flags */
    bool echo_;
    int samples_;
    bool dynamic_data_ = false;
    int forced_domain_;

    /* Static Types */
    LatencyDataType latency_data_type_;
    LatencyType* latency_type_;
    TestCommandDataType latency_command_type_;

    /* Dynamic Types */
    eprosima::fastrtps::types::DynamicData* dynamic_data_type_;
    eprosima::fastrtps::types::DynamicPubSubType dynamic_data_pub_sub_type_;
    eprosima::fastrtps::types::DynamicType_ptr dynamic_type_;

    /* Data Listeners */
    class DataPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:

        DataPubListener(
                LatencyTestSubscriber* latency_publisher)
            : latency_publisher_(latency_publisher)
        {
        }

        ~DataPubListener()
        {
        }

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        LatencyTestSubscriber* latency_publisher_;
    } data_pub_listener_;

    class DataSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        DataSubListener(
                LatencyTestSubscriber* latency_publisher)
            : latency_publisher_(latency_publisher)
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

        LatencyTestSubscriber* latency_publisher_;
    } data_sub_listener_;

    /* Command Listeners */
    class CommandPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:

        CommandPubListener(
                LatencyTestSubscriber* latency_publisher)
            : latency_publisher_(latency_publisher)
        {
        }

        ~CommandPubListener()
        {
        }

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* pub,
                eprosima::fastrtps::rtps::MatchingInfo& info);

        LatencyTestSubscriber* latency_publisher_;
    } command_pub_listener_;

    class CommandSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        CommandSubListener(
                LatencyTestSubscriber* latency_publisher)
            : latency_publisher_(latency_publisher)
        {
        }

        ~CommandSubListener()
        {
        }

        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber* sub,
                eprosima::fastrtps::rtps::MatchingInfo& into);

        void onNewDataMessage(
                eprosima::fastrtps::Subscriber* sub);

        LatencyTestSubscriber* latency_publisher_;
    } command_sub_listener_;
};

#endif /* LATENCYTESTSUBSCRIBER_H_ */
