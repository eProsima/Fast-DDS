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
    // Initialize internal variables
    matched_ = 0;

    // Set DomainParticipant name
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");
    // Create DomainParticipant
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (nullptr == participant_)
    {
        return false;
    }

    // If using the custom filter
    if (custom_filter)
    {
        // Register the filter factory
        if (ReturnCode_t::RETCODE_OK !=
                participant_->register_content_filter_factory("MY_CUSTOM_FILTER", &filter_factory))
        {
            return false;
        }
    }

    // Register the type
    type_.register_type(participant_);

    // Create the Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (nullptr == subscriber_)
    {
        return false;
    }

    // Create the Topic
    topic_ = participant_->create_topic("HelloWorldTopic", type_->getName(), TOPIC_QOS_DEFAULT);
    if (nullptr == topic_)
    {
        return false;
    }

    // Create the ContentFilteredTopic
    std::string expression;
    std::vector<std::string> parameters;
    if (custom_filter)
    {
        // Custom filter: reject samples where index > parameters[0] and index < parameters[1].
        // Custom filter does not use expression. However, an empty expression disables filtering, so some expression
        // must be set.
        expression = " ";
        parameters.push_back("3");
        parameters.push_back("5");
        filter_topic_ =
                participant_->create_contentfilteredtopic("HelloWorldFilteredTopic1", topic_, expression, parameters,
                        "MY_CUSTOM_FILTER");
    }
    else
    {
        // Default filter: accept samples meeting the given expression: index between the two given parameters
        expression = "index between %0 and %1";
        parameters.push_back("5");
        parameters.push_back("9");
        filter_topic_ =
                participant_->create_contentfilteredtopic("HelloWorldFilteredTopic1", topic_, expression, parameters);
    }
    if (nullptr == filter_topic_)
    {
        return false;
    }

    // Create the DataReader
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    // In order to receive all samples, DataReader is configured as RELIABLE and TRANSIENT_LOCAL (ensure reception even
    // if DataReader matching is after DataWriter one)
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
    // Delete DDS entities contained within the DomainParticipant
    participant_->delete_contained_entities();
    // Delete DomainParticipant
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void ContentFilteredTopicExampleSubscriber::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    // New remote DataWriter discovered
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    // New remote DataWriter undiscovered
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    // Non-valid option
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
    // Take next sample from DataReader's history
    if (ReturnCode_t::RETCODE_OK == reader->take_next_sample(&hello_, &info))
    {
        // Some samples only update the instance state. Only if it is a valid sample (with data)
        if (ALIVE_INSTANCE_STATE == info.instance_state)
        {
            samples_++;
            // Print structure data
            std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" << std::endl;
        }
    }
}

void ContentFilteredTopicExampleSubscriber::run()
{
    // Subscriber application thread running until stopped by the user
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}
