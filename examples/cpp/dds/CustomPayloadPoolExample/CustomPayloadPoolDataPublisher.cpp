// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file CustomPayloadPoolDataPublisher.cpp
 *
 */

#include <thread>
#include <csignal>

#include "CustomPayloadPoolDataPublisher.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>


using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> CustomPayloadPoolDataPublisher::stop_(false);
std::mutex CustomPayloadPoolDataPublisher::wait_matched_cv_mtx_;
std::condition_variable CustomPayloadPoolDataPublisher::wait_matched_cv_;

CustomPayloadPoolDataPublisher::CustomPayloadPoolDataPublisher(
        std::shared_ptr<CustomPayloadPool> payload_pool)
    : payload_pool_(payload_pool)
    , participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new CustomPayloadPoolDataPubSubType())
    , matched_(0)
    , has_stopped_for_unexpected_error_(false)
{
}

bool CustomPayloadPoolDataPublisher::is_stopped()
{
    return stop_;
}

void CustomPayloadPoolDataPublisher::stop()
{
    stop_ = true;
    awake();
}

bool CustomPayloadPoolDataPublisher::init()
{
    hello_.index(0);
    hello_.message("CustomPayloadPool");
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("CustomPayloadPoolDataPublisher");
    auto factory = DomainParticipantFactory::get_instance();

    participant_ = factory->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    /* Register the type */
    type_.register_type(participant_);

    /* Create the publisher */
    publisher_ = participant_->create_publisher(
        PUBLISHER_QOS_DEFAULT,
        nullptr);

    if (publisher_ == nullptr)
    {
        return false;
    }

    /* Create the topic */
    topic_ = participant_->create_topic(
        "CustomPayloadPoolTopic",
        type_.get_type_name(),
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    /* Create the writer */
    writer_ = publisher_->create_datawriter(
        topic_,
        DATAWRITER_QOS_DEFAULT,
        this,
        StatusMask::all(),
        payload_pool_);

    if (writer_ == nullptr)
    {
        return false;
    }

    // Register SIGINT signal handler to stop thread execution
    signal(SIGINT, [](int /*signum*/)
            {
                std::cout << "SIGINT received, stopping Publisher execution." << std::endl;
                CustomPayloadPoolDataPublisher::stop();
            });

    return true;
}

CustomPayloadPoolDataPublisher::~CustomPayloadPoolDataPublisher()
{
    if (participant_ != nullptr)
    {
        participant_->delete_contained_entities();
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void CustomPayloadPoolDataPublisher::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher matched." << std::endl;
        if (matched_ > 0)
        {
            awake();
        }
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void CustomPayloadPoolDataPublisher::wait()
{
    std::unique_lock<std::mutex> lck(wait_matched_cv_mtx_);
    wait_matched_cv_.wait(lck, [this]
            {
                return matched_ > 0 || is_stopped();
            });
}

void CustomPayloadPoolDataPublisher::awake()
{
    wait_matched_cv_.notify_all();
}

void CustomPayloadPoolDataPublisher::run_thread(
        uint32_t samples,
        uint32_t sleep)
{
    while (!is_stopped() && (samples == 0 || hello_.index() < samples))
    {
        if (matched_ > 0)
        {
            if (publish())
            {
                std::cout << "Message: " << hello_.message().data() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
            }
            // something went wrong writing
            else
            {
                has_stopped_for_unexpected_error_ = true;
                CustomPayloadPoolDataPublisher::stop();
            }
        }
        else
        {
            wait();
        }
    }
}

bool CustomPayloadPoolDataPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop_ = false;
    std::thread thread(&CustomPayloadPoolDataPublisher::run_thread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }

    thread.join();
    return has_stopped_for_unexpected_error_;
}

bool CustomPayloadPoolDataPublisher::publish()
{
    hello_.index(hello_.index() + 1);
    stop_ = !writer_->write(&hello_);
    return !stop_;
}
