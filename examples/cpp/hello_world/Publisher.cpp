// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Publisher.cpp
 *
 */

#include "Publisher.hpp"

#include <condition_variable>
#include <csignal>
#include <stdexcept>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

using namespace eprosima::fastdds::dds;

std::atomic<bool> HelloWorldPublisher::stop_(false);
std::condition_variable HelloWorldPublisher::matched_cv_;

HelloWorldPublisher::HelloWorldPublisher(
        const CLIParser::hello_world_config& config)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new HelloWorldPubSubType())
{
    // Set up the data type with initial values
    hello_.index(0);
    hello_.message("Hello world");
    matched_ = 0;

    // Get CLI options
    samples_ = config.samples;

    // Create the participant
    auto factory = DomainParticipantFactory::get_instance();
    participant_ = factory->create_participant_with_default_profile(nullptr, StatusMask::none());
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Register the type
    type_.register_type(participant_);

    // Create the publisher
    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    participant_->get_default_publisher_qos(pub_qos);
    publisher_ = participant_->create_publisher(pub_qos, nullptr, StatusMask::none());
    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic("hello_world_topic", type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the data writer
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    publisher_->get_default_datawriter_qos(writer_qos);
    writer_ = publisher_->create_datawriter(topic_, writer_qos, this, StatusMask::all());
    if (writer_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    // Delete DDS entities contained within the DomainParticipant
    participant_->delete_contained_entities();

    // Delete DomainParticipant
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldPublisher::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher matched." << std::endl;
        matched_cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldPublisher::run()
{
    std::thread pub_thread([&]
            {
                while (!is_stopped() && (samples_ == 0 || hello_.index() < samples_))
                {
                    if (publish())
                    {
                        std::cout << "Message: '" << hello_.message() << "' with index: '" << hello_.index()
                                  << "' SENT" << std::endl;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(period_));
                }
            });
    if (samples_ == 0)
    {
        std::cout << "Publisher running. Please press Ctrl+C to stop the Publisher at any time." << std::endl;
    }
    else
    {
        std::cout << "Publisher running " << samples_ <<
            " samples. Please press Ctrl+C to stop the Publisher at any time." << std::endl;
    }
    signal(SIGINT, [](int /*signum*/)
            {
                std::cout << "\nSIGINT received, stopping Publisher execution." << std::endl;
                HelloWorldPublisher::stop();
            });
    signal(SIGTERM, [](int /*signum*/)
            {
                std::cout << "\nSIGTERM received, stopping Publisher execution." << std::endl;
                HelloWorldPublisher::stop();
            });
#ifndef _WIN32
    signal(SIGQUIT, [](int /*signum*/)
            {
                std::cout << "\nSIGQUIT received, stopping Publisher execution." << std::endl;
                HelloWorldPublisher::stop();
            });
    signal(SIGHUP, [](int /*signum*/)
            {
                std::cout << "\nSIGHUP received, stopping Publisher execution." << std::endl;
                HelloWorldPublisher::stop();
            });
#endif // _WIN32
    pub_thread.join();
}

bool HelloWorldPublisher::publish()
{
    bool ret = false;
    // Wait for the data endpoints discovery
    std::unique_lock<std::mutex> matched_lock(mutex_);
    matched_cv_.wait(matched_lock, [&]()
            {
                // at least one has been discovered
                return matched_ > 0 || is_stopped();
            });
    if (!is_stopped())
    {
        hello_.index(hello_.index() + 1);
        ret = writer_->write(&hello_);
    }
    return ret;
}

bool HelloWorldPublisher::is_stopped()
{
    return stop_.load();
}

void HelloWorldPublisher::stop()
{
    stop_.store(true);
    matched_cv_.notify_one();
}
