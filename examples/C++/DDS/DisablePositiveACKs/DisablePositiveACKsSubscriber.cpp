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
 * @file DisablePositiveACKsSubscriber.cpp
 *
 */

#include "DisablePositiveACKsSubscriber.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

DisablePositiveACKsSubscriber::DisablePositiveACKsSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , reader_(nullptr)
    , topic_(nullptr)
    , type_(new TopicPubSubType())
{
}

bool DisablePositiveACKsSubscriber::init(
        bool disable_positive_acks)
{
    // Create Participant
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register type
    type_.register_type(participant_);

    // Create Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    // Create Topic
    topic_ = participant_->create_topic("DisablePositiveACKsTopic", "Topic", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // Create datareader
    DataReaderQos rqos;
    rqos.history().kind = KEEP_ALL_HISTORY_QOS;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    rqos.reliable_reader_qos().disable_positive_ACKs.enabled = disable_positive_acks;

    reader_ = subscriber_->create_datareader(topic_, rqos, &listener);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

DisablePositiveACKsSubscriber::~DisablePositiveACKsSubscriber()
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

void DisablePositiveACKsSubscriber::SubListener::on_subscription_matched(
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

void DisablePositiveACKsSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (reader->take_next_sample(&hello, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            this->n_samples++;
            std::cout << "Message with index " << hello.index() << " RECEIVED" << std::endl;
        }
    }
}

void DisablePositiveACKsSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << " samples have been received" << std::endl;
    while (number > this->listener.n_samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
