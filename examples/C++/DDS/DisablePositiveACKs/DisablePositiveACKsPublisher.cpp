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
 * @file DisablePositiveACKsPublisher.cpp
 *
 */

#include "DisablePositiveACKsPublisher.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <thread>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

DisablePositiveACKsPublisher::DisablePositiveACKsPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new TopicType())
{
}

bool DisablePositiveACKsPublisher::init(
        bool disable_positive_acks,
        uint32_t keep_duration_ms)
{
    hello_.index(0);
    hello_.message("DisablePositiveACKs");

    // Create Participant
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register type
    type_.register_type(participant_);

    // Create Publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (publisher_ == nullptr)
    {
        return false;
    }

    // Create Topic
    topic_ = participant_->create_topic("DisablePositiveACKsTopic", "Topic", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // Create DataWriter
    DataWriterQos wqos;
    wqos.history().kind = KEEP_ALL_HISTORY_QOS;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    wqos.reliable_writer_qos().disable_positive_acks.enabled = disable_positive_acks;
    wqos.reliable_writer_qos().disable_positive_acks.duration = eprosima::fastrtps::Duration_t(keep_duration_ms * 1e-3);

    writer_ = publisher_->create_datawriter(topic_, wqos, &listener);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;
}

DisablePositiveACKsPublisher::~DisablePositiveACKsPublisher()
{
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }
    if (publisher_ != nullptr)
    {
        participant_->delete_publisher(publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void DisablePositiveACKsPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void DisablePositiveACKsPublisher::run(
        uint32_t samples,
        uint32_t write_sleep_ms)
{
    std::cout << "Publisher running" << std::endl;

    samples = ( samples == 0 ) ? 10 : samples;
    for ( uint32_t i = 0; i < samples; ++i )
    {
        if ( !publish() )
        {
            --i;
        }
        else
        {
            std::cout << "Message with index: " << hello_.index() << " SENT" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(write_sleep_ms));
    }

    std::cout << "Please press enter to stop the Publisher" << std::endl;
    std::cin.ignore();
}

bool DisablePositiveACKsPublisher::publish(
        bool waitForListener)
{
    if (!waitForListener || listener.n_matched > 0)
    {
        hello_.index(hello_.index() + 1);
        writer_->write((void*)&hello_);
        return true;
    }
    return false;
}
