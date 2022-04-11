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
 * @file ContentFilteredTopicExamplePublisher.cpp
 *
 */

#include "ContentFilteredTopicExamplePublisher.hpp"

#include <thread>

#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

using namespace eprosima::fastdds::dds;

bool ContentFilteredTopicExamplePublisher::init()
{
    // Initialize internal variables
    matched_ = 0;

    // Initialize data sample
    hello_.index(0);
    hello_.message("HelloWorld");

    // Set DomainParticipant name
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    // Create DomainParticipant in domain 0
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (nullptr == participant_)
    {
        return false;
    }

    // Register the type
    type_.register_type(participant_);

    // Create the Publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (nullptr == publisher_)
    {
        return false;
    }

    // Create the Topic
    topic_ = participant_->create_topic("HelloWorldTopic", type_->getName(), TOPIC_QOS_DEFAULT);
    if (nullptr == topic_)
    {
        return false;
    }

    // Create the DataWriter
    writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, this);
    if (nullptr == writer_)
    {
        return false;
    }
    return true;
}

ContentFilteredTopicExamplePublisher::~ContentFilteredTopicExamplePublisher()
{
    // Delete DDS entities contained within the DomainParticipant
    participant_->delete_contained_entities();
    // Delete DomainParticipant
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void ContentFilteredTopicExamplePublisher::on_publication_matched(
        DataWriter*,
        const PublicationMatchedStatus& info)
{
    // New remote DataReader discovered
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        firstConnected_ = true;
        std::cout << "Publisher matched." << std::endl;
    }
    // New remote DataReader undiscovered
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    // Non-valid option
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void ContentFilteredTopicExamplePublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    // Publish samples continously until stopped by the user
    if (samples == 0)
    {
        while (!stop_)
        {
            if (publish(false))
            {
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
    // Publish given number of samples
    else
    {
        for (uint32_t i = 0; i < samples; ++i)
        {
            if (!publish())
            {
                --i;
            }
            else
            {
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
}

void ContentFilteredTopicExamplePublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    // Spawn publisher application thread
    stop_ = false;
    std::thread thread(&ContentFilteredTopicExamplePublisher::runThread, this, samples, sleep);
    // Thread runs indefinitely until stopped by the user
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop_ = true;
    }
    // Thread runs only for the given samples
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

bool ContentFilteredTopicExamplePublisher::publish(
        bool wait_for_listener)
{
    // Wait until there is a matched DataReader, unless wait_for_listener flag is set to false
    if (first_connected_ || !wait_for_listener || matched_ > 0)
    {
        // Update sample
        hello_.index(hello_.index() + 1);
        // Write sample
        writer_->write(&hello_);
        return true;
    }
    return false;
}
