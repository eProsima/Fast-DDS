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
 * @file LivelinessSubscriber.cpp
 *
 */

#include "LivelinessSubscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

LivelinessSubscriber::LivelinessSubscriber()
    : type_(new TopicPubSubType())
    , participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
{
}

bool LivelinessSubscriber::init(
        LivelinessQosPolicyKind kind,
        int liveliness_ms)
{
    //Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SIMPLE;
    pqos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    pqos.wire_protocol().builtin.use_WriterLivelinessProtocol = true;
    pqos.name("Participant_sub");

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    //Register Type
    type_.register_type(participant_);

    //Create Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    //Create Topic
    topic_ = participant_->create_topic("Name", "Topic", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    //Create DataReader
    DataReaderQos rqos;
    rqos.history().depth = 30;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.liveliness().lease_duration = Duration_t(liveliness_ms * 1e-3);
    rqos.liveliness().announcement_period = Duration_t(liveliness_ms * 1e-3 * 0.5);
    rqos.liveliness().kind = kind;

    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

LivelinessSubscriber::~LivelinessSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void LivelinessSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void LivelinessSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (reader->take_next_sample(&topic, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == eprosima::fastdds::dds::ALIVE)
        {
            this->n_samples++;

            std::cout << "Message with index " << topic.index() << " RECEIVED" << std::endl;
        }
    }
}

void LivelinessSubscriber::SubListener::on_liveliness_changed(
        DataReader*,
        const LivelinessChangedStatus& status)
{
    if (status.alive_count_change == 1)
    {
        std::cout << "Publisher " << status.last_publication_handle << " recovered liveliness" << std::endl;
    }
    else if (status.not_alive_count_change == 1)
    {
        std::cout << "Publisher " << status.last_publication_handle << " lost liveliness" << std::endl;
    }
}

void LivelinessSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void LivelinessSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << "samples have been received" << std::endl;

    while (number > this->listener_.n_samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void LivelinessSubscriber::PartListener::on_participant_discovery(
        DomainParticipant*,
        ParticipantDiscoveryInfo&& info)
{
    if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Participant discovered" << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        std::cout << "Participant dropped" << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Participant removed" << std::endl;
    }
}
