// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LivelinessPublisher.cpp
 *
 */

#include "LivelinessPublisher.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <thread>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

LivelinessPublisher::LivelinessPublisher()
    : type_(new TopicType())
    , participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)

{
}

bool LivelinessPublisher::init(
        LivelinessQosPolicyKind kind,
        int liveliness_ms)
{
    topic.index(0);
    topic.message("HelloWorld");

    //Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SIMPLE;
    pqos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    pqos.wire_protocol().builtin.use_WriterLivelinessProtocol = true;
    pqos.name("Participant_pub");

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    //Register Type
    type_.register_type(participant_);

    //Create Publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (publisher_ == nullptr)
    {
        return false;
    }

    //Create Topic
    topic_ = participant_->create_topic("Name", "Topic", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    //Create DataWriter
    DataWriterQos wqos;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 30;
    wqos.liveliness().lease_duration = eprosima::fastrtps::Duration_t(liveliness_ms * 1e-3);
    wqos.liveliness().announcement_period = eprosima::fastrtps::Duration_t(liveliness_ms * 1e-3 * 0.5);
    wqos.liveliness().kind = kind;

    writer_ = publisher_->create_datawriter(topic_, wqos, &listener_);
    if (writer_ == nullptr)
    {
        return false;
    }
    return true;
}

LivelinessPublisher::~LivelinessPublisher()
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

void LivelinessPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        first_connected = true;
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

void LivelinessPublisher::PubListener::on_liveliness_lost(
        DataWriter* writer,
        const LivelinessLostStatus& status)
{
    std::cout << "Publisher " << writer->guid() << " lost liveliness: " << status.total_count << std::endl;
}

void LivelinessPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    std::thread thread1(&LivelinessPublisher::runThread, this, writer_, samples, sleep);
    thread1.join();
}

void LivelinessPublisher::runThread(
        DataWriter* writer,
        uint32_t samples,
        uint32_t sleep)
{

    for (uint32_t i = 0; i < samples; ++i)
    {
        if (!publish(writer))
        {
            --i;
        }
        else
        {
            std::cout << "Message with index: " << topic.index() << " SENT by publisher " << writer->guid() <<
                std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }

    std::cin.ignore();
}

bool LivelinessPublisher::publish(
        DataWriter* writer,
        bool waitForListener)
{
    if (listener_.first_connected || !waitForListener || listener_.n_matched > 0)
    {
        topic.index(topic.index() + 1);
        writer->write((void*)&topic);
        return true;
    }
    return false;
}
