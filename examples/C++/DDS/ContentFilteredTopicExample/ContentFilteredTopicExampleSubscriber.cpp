// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ContentFilteredTopicExampleSubscriber.cpp
 *
 */

#include "ContentFilteredTopicExampleSubscriber.hpp"

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

using namespace eprosima::fastdds::dds;

bool ContentFilteredTopicExampleSubscriber::init(
        bool custom_filter)
{
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (nullptr == participant_)
    {
        return false;
    }

    if (custom_filter)
    {
        // Register the filter factory
        if (ReturnCode_t::RETCODE_OK !=
                participant_->register_content_filter_factory("MY_CUSTOM_FILTER", &filter_factory))
        {
            // Error
            return false;
        }
    }

    //Register the type
    type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (nullptr == subscriber_)
    {
        return false;
    }

    //Create the topic
    topic_ = participant_->create_topic("HelloWorldTopic", type_->getName(), TOPIC_QOS_DEFAULT);

    if (nullptr == topic_)
    {
        return false;
    }

    // Create ContentFilteredTopic
    std::string expression;
    std::vector<std::string> parameters;
    if (custom_filter)
    {
        // Custom filter: reject samples where index > parameters[0] and index < parameters[1].
        expression = " ";
        parameters.push_back("3");
        parameters.push_back("5");
        filter_topic_ =
                participant_->create_contentfilteredtopic("HelloWorldFilteredTopic1", topic_, expression, parameters,
                        "MY_CUSTOM_FILTER");
    }
    else
    {
        expression = "index between %0 and %1";
        parameters.push_back("5");
        parameters.push_back("9");
        filter_topic_ =
                participant_->create_contentfilteredtopic("HelloWorldFilteredTopic1", topic_, expression, parameters);
    }

    if (nullptr == filter_topic_)
    {
        // Error
        return false;
    }

    // CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_ = subscriber_->create_datareader(filter_topic_, rqos, this);

    if (nullptr == reader_)
    {
        return false;
    }

    return true;
}

ContentFilteredTopicExampleSubscriber::~ContentFilteredTopicExampleSubscriber()
{
    participant_->delete_contained_entities();
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void ContentFilteredTopicExampleSubscriber::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void ContentFilteredTopicExampleSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (ReturnCode_t::RETCODE_OK == reader->take_next_sample(&hello_, &info))
    {
        if (ALIVE_INSTANCE_STATE == info.instance_state)
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" << std::endl;
        }
    }
}

void ContentFilteredTopicExampleSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}
