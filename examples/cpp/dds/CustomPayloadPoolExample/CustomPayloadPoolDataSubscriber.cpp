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
 * @file CustomPayloadPoolDataSubscriber.cpp
 *
 */

#include <csignal>

#include "CustomPayloadPoolDataSubscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

using namespace eprosima::fastdds::dds;

std::atomic<bool> CustomPayloadPoolDataSubscriber::stop_(false);
std::mutex CustomPayloadPoolDataSubscriber::terminate_cv_mtx_;
std::condition_variable CustomPayloadPoolDataSubscriber::terminate_cv_;

CustomPayloadPoolDataSubscriber::CustomPayloadPoolDataSubscriber(
        std::shared_ptr<CustomPayloadPool> payload_pool)
    : payload_pool_(payload_pool)
    , participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new CustomPayloadPoolDataPubSubType())
    , matched_(0)
    , samples_(0)
    , max_samples_(0)
{

}

bool CustomPayloadPoolDataSubscriber::is_stopped()
{
    return stop_;
}

void CustomPayloadPoolDataSubscriber::stop()
{
    stop_ = true;
    terminate_cv_.notify_all();
}

bool CustomPayloadPoolDataSubscriber::init()
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("CustomPayloadPoolDataSubscriber");
    auto factory = DomainParticipantFactory::get_instance();

    participant_ = factory->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    /* Register the type */
    type_.register_type(participant_);

    /* Create the subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
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

    /* Create the reader */
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    reader_ = subscriber_->create_datareader(topic_, rqos, this, StatusMask::all(), payload_pool_);

    if (reader_ == nullptr)
    {
        return false;
    }

    // Register SIGINT signal handler to stop thread execution
    signal(SIGINT, [](int /*signum*/)
            {
                std::cout << "SIGINT received, stopping subscriber execution." << std::endl;
                CustomPayloadPoolDataSubscriber::stop();
            });

    return true;
}

CustomPayloadPoolDataSubscriber::~CustomPayloadPoolDataSubscriber()
{
    if (participant_ != nullptr)
    {
        participant_->delete_contained_entities();
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void CustomPayloadPoolDataSubscriber::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void CustomPayloadPoolDataSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message [" << samples_ << "] of " << hello_.message() << " " << hello_.index()
                      << " RECEIVED" << std::endl;

            if (max_samples_ > 0 && (samples_ >= max_samples_))
            {
                stop();
            }
        }
    }
}

bool CustomPayloadPoolDataSubscriber::run(
        uint32_t samples)
{
    max_samples_ = samples;
    stop_ = false;
    if (samples == 0)
    {
        std::cout << "Subscriber running. Please press Ctrl+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Subscriber running until " << samples << " samples have been received" << std::endl;
    }

    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
    return is_stopped();
}
