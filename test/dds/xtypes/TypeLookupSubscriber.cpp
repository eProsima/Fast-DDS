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
 * @file TypeLookupSubscriber.cpp
 *
 */

#include "TypeLookupSubscriber.h"

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

static int DOMAIN_ID_ = 10;

TypeLookupSubscriber::~TypeLookupSubscriber()
{
    if (nullptr != reader_)
    {
        subscriber_->delete_datareader(reader_);
    }

    if (nullptr != subscriber_)
    {
        participant_->delete_subscriber(subscriber_);
    }

    if (nullptr != topic_)
    {
        participant_->delete_topic(topic_);
    }

    if (nullptr != participant_)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool TypeLookupSubscriber::init()
{
    std::cout << "TypeLookupSubscriber::init" << std::endl;

    StatusMask mask = StatusMask::subscription_matched()
            << StatusMask::data_available()
            << StatusMask::liveliness_changed();

    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(DOMAIN_ID_, PARTICIPANT_QOS_DEFAULT, this, mask);

    if (participant_ == nullptr)
    {
        std::cout << "ERROR create_participant" << std::endl;
        return false;
    }

    if (!create_type())
    {
        std::cout << "ERROR create_type" << std::endl;
        return false;
    }

    return true;
}

bool TypeLookupSubscriber::create_type()
{
    auto obj = BasicStruct();
    type_.reset(new BasicStructPubSubType());
    type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        std::cout << "ERROR create_subscriber" << std::endl;
        return false;
    }

    //CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << "BasicStructPubSubType" << "_" << asio::ip::host_name() << "_" << DOMAIN_ID_;
    topic_ = participant_->create_topic(topic_name.str(), type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        std::cout << "ERROR create_topic" << std::endl;
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos = subscriber_->get_default_datareader_qos();
    reader_ = subscriber_->create_datareader(topic_, rqos);
    if (reader_ == nullptr)
    {
        std::cout << "ERROR create_datareader" << std::endl;
        return false;
    }

    return true;
}

bool TypeLookupSubscriber::run()
{
    return run_for(std::chrono::seconds(30));
}

bool TypeLookupSubscriber::run_for(
        const std::chrono::milliseconds& timeout)
{
    bool returned_value = false;

    std::unique_lock<std::mutex> lock(mutex_);
    returned_value = cv_.wait_for(lock, timeout, [&]
                    {
                        if (publishers_ < number_samples_.size())
                        {
                            // Will fail later.
                            return true;
                        }
                        else if (publishers_ > number_samples_.size())
                        {
                            return false;
                        }

                        for (auto& number_samples : number_samples_)
                        {
                            if (max_number_samples_ > number_samples.second)
                            {
                                return false;
                            }
                        }

                        return true;
                    });

    if (publishers_ < number_samples_.size())
    {
        std::cout << "ERROR: detected more than " << publishers_ << " publishers" << std::endl;
        returned_value = false;
    }

    return returned_value;
}

void TypeLookupSubscriber::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "(TypeLookupSubscriber) matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "(TypeLookupSubscriber) unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void TypeLookupSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    BasicStruct sample;
    if (reader->take_next_sample((void*)&sample, &info) == RETCODE_OK)
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            std::cout << "Received sample (" << info.sample_identity.writer_guid().guidPrefix << " - " <<
                info.sample_identity.sequence_number() << "): index(" << sample.index() << "), message("
                      << sample.message() << ")" << std::endl;
            if (max_number_samples_ <= ++number_samples_[info.sample_identity.writer_guid()])
            {
                cv_.notify_all();
            }
        }
    }
}
