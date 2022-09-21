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
 * @file BasicSubscriber.cpp
 *
 */

#include <csignal>
#include <exception>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include "BasicSubscriber.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> BasicSubscriber::stop_(false);
std::mutex BasicSubscriber::terminate_cv_mtx_;
std::condition_variable BasicSubscriber::terminate_cv_;

bool BasicSubscriber::is_stopped()
{
    return stop_;
}

void BasicSubscriber::stop()
{
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    stop_ = true;
    terminate_cv_.notify_all();
}

BasicSubscriber::BasicSubscriber(
        const std::string& topic_name,
        uint32_t max_messages,
        uint32_t domain)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType())
    , samples_(0)
    , max_messages_(max_messages)
{
    // CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.name("BasicSubscriber");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Error creating Participant.");
    }

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Error creating Subscriber.");
    }

    // CREATE THE TOPIC
    topic_ = participant_->create_topic(
        topic_name,
        type_.get_type_name(),
        TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Error creating Topic.");
    }

    // CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    reader_ = subscriber_->create_datareader(topic_, rqos, this);
    if (reader_ == nullptr)
    {
        throw std::runtime_error("Error creating DataReader.");
    }
}

BasicSubscriber::~BasicSubscriber()
{
    // In case it has not stopped yet, set Reader as stopped so thread can finish
    stop();

    if (participant_ != nullptr)
    {
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            if (reader_ != nullptr)
            {
                subscriber_->delete_datareader(reader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void BasicSubscriber::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cerr << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void BasicSubscriber::read_()
{
    SampleInfo info;
    while ((reader_->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message " << hello_.message().data() << " " << hello_.index() << " RECEIVED" << std::endl;
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void BasicSubscriber::on_data_available(
        DataReader* )
{
    // This will cause that reception thread gets busy receiving and reading
    // Thus it will make reception slower
    if (!is_stopped())
    {
        read_();
    }
}

void BasicSubscriber::run(
        uint32_t samples)
{
    // Setting as not stopped
    stop_ = false;

    // Setting signal handler
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Subscriber execution." << std::endl;
                static_cast<void>(signum); BasicSubscriber::stop();
            });

    // Printing output message
    if (samples > 0)
    {
        std::cout << "Basic Subscriber running until " << samples <<
            " samples have been received. Please press CTRL+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Basic Subscriber running. Please press CTRL+C to stop the Subscriber." << std::endl;
    }

    // Wait to finish by enough messages received or signal
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
