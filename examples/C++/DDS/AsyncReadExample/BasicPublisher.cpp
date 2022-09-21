// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BasicPublisher.cpp
 *
 */

#include <csignal>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include "BasicPublisher.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> BasicPublisher::stop_(false);

bool BasicPublisher::is_stopped()
{
    return stop_;
}

void BasicPublisher::stop()
{
    stop_ = true;
}

BasicPublisher::BasicPublisher(
        const std::string& topic_name,
        uint32_t domain)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new HelloWorldPubSubType())
{
    // INITIALIZE DATA TO SEND
    hello_.index(0);
    memcpy(hello_.message().data(), "HelloWorld ", strlen("HelloWorld") + 1);

    // CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Error creating Participant.");
    }

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Error creating Publisher.");
    }

    // CREATE THE TOPIC
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Error creating Topic.");
    }

    // CREATE THE WRITER
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    writer_ = publisher_->create_datawriter(topic_, wqos, this);
    if (writer_ == nullptr)
    {
        throw std::runtime_error("Error creating DataWriter.");
    }
}

BasicPublisher::~BasicPublisher()
{
    // In case it has not stopped yet, set Reader as stopped so thread can finish
    stop();

    if (participant_ != nullptr)
    {
        if (publisher_ != nullptr)
        {
            if (writer_ != nullptr)
            {
                publisher_->delete_datawriter(writer_);
            }
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void BasicPublisher::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cerr << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void BasicPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    // Setting as not stopped
    stop_ = false;

    // Setting signal handler
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Publisher execution." << std::endl;
                static_cast<void>(signum); BasicPublisher::stop();
            });

    // Printing output message
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }
    else
    {
        std::cout << "Publisher running " << samples <<
            " samples. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }

    // Publish until finish or stop
    while (!is_stopped() && (samples == 0 || hello_.index() < samples))
    {
        publish_();
        std::cout << "Message: " << hello_.message().data() << " with index: " << hello_.index()
                    << " SENT" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}

void BasicPublisher::publish_()
{
    hello_.index(hello_.index() + 1);
    writer_->write(&hello_);
}
