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
 * @file LifespanSubscriber.cpp
 *
 */

#include "LifespanSubscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

LifespanSubscriber::LifespanSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new LifespanPubSubType())
{
}

bool LifespanSubscriber::init(
        uint32_t lifespan_ms)
{
    //Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
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
    topic_ = participant_->create_topic("LifespanTopic", "Lifespan", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    //Create DataReader
    DataReaderQos rqos;
    rqos.history().kind = KEEP_ALL_HISTORY_QOS;
    rqos.history().depth = 30;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    rqos.lifespan().duration = lifespan_ms * 1e-3;

    reader_ = subscriber_->create_datareader(topic_, rqos, &listener);
    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

LifespanSubscriber::~LifespanSubscriber()
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

void LifespanSubscriber::SubListener::on_subscription_matched(
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

void LifespanSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (reader->read_next_sample(&hello, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            this->n_samples++;
            // Print your structure data here.
            std::cout << "Message " << hello.message() << " " << hello.index() << " RECEIVED" << std::endl;
        }
    }
}

void LifespanSubscriber::run(
        uint32_t number,
        uint32_t sleep_ms)
{
    std::cout << "Subscriber running until " << number << " samples have been received" << std::endl;
    while ( number > this->listener.n_samples )
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Now wait and try to remove from history
    std::cout << std::endl << "Subscriber waiting for " << sleep_ms << " milliseconds" << std::endl << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));

    LifespanPubSubType::type data;
    SampleInfo info;

    for ( uint32_t i = 0; i < listener.n_samples; i++ )
    {
        if (reader_->take_next_sample((void*) &data, &info) == ReturnCode_t::RETCODE_OK)
        {
            std::cout << "Message " << data.message() << " " << data.index() << " read from history" << std::endl;
        }
        else
        {
            std::cout << "Could not read message " << i << " from history" << std::endl;
        }
    }
}
