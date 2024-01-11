// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    for (auto it = known_types_.begin(); it != known_types_.end(); ++it)
    {
        it->second.subscriber_->delete_datareader(it->second.reader_);
        participant_->delete_subscriber(it->second.subscriber_);
        participant_->delete_topic(it->second.topic_);
    }

    if (nullptr != participant_)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool TypeLookupSubscriber::init(
        std::vector<std::string> known_types)
{
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

    for (const auto& type : known_types)
    {
        if (type == "Type1")
        {
            if (!create_known_type<Type1, Type1PubSubType>(type))
            {
                std::cout << "ERROR create_known_type: " << type << std::endl;
                return false;
            }
        }
        else if (type == "Type2")
        {
            if (!create_known_type<Type2, Type2PubSubType>(type))
            {
                std::cout << "ERROR create_known_type: " << type << std::endl;
                return false;
            }
        }
        else if (type == "Type3")
        {
            if (!create_known_type<Type3, Type3PubSubType>(type))
            {
                std::cout << "ERROR create_known_type: " << type << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "ERROR TypeLookupSubscriber::init uknown type: " << type << std::endl;
            return false;
        }
    }
    return true;
}

template <typename Type, typename TypePubSubType>
bool TypeLookupSubscriber::create_known_type(
        const std::string& type)
{
    // Check if the type is already created
    if (known_types_.find(type) != known_types_.end())
    {
        std::cout << "Type " << type << " is already created." << std::endl;
        return true;
    }

    SubKnownType a_type;
    a_type.obj_ = new Type();
    a_type.type_.reset(new TypePubSubType());
    a_type.type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    a_type.subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (a_type.subscriber_ == nullptr)
    {
        std::cout << "ERROR create_subscriber" << std::endl;
        return false;
    }

    //CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << type << "_" << asio::ip::host_name() << "_" << DOMAIN_ID_;
    a_type.topic_ = participant_->create_topic(topic_name.str(), a_type.type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (a_type.topic_ == nullptr)
    {
        std::cout << "ERROR create_topic" << std::endl;
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos = a_type.subscriber_->get_default_datareader_qos();
    a_type.reader_ = a_type.subscriber_->create_datareader(a_type.topic_, rqos);
    if (a_type.reader_ == nullptr)
    {
        std::cout << "ERROR create_datareader" << std::endl;
        return false;
    }

    // CREATE CALLBACK
    a_type.callback_ = [](void* data)
            {
                Type* sample = static_cast<Type*>(data);
                std::cout <<  "index(" << sample->index() << "), message(" << sample->message() << ")" << std::endl;
            };

    known_types_.emplace(type, a_type);
    return true;
}

bool TypeLookupSubscriber::run(
        int seconds)
{
    return run_for(std::chrono::seconds(seconds));
}

bool TypeLookupSubscriber::run_for(
        const std::chrono::milliseconds& timeout)
{
    bool returned_value = false;

    std::unique_lock<std::mutex> lock(mutex_);
    returned_value = cv_.wait_for(lock, timeout, [&]
                    {
                        if (known_types_.size() < number_samples_.size())
                        {
                            // Will fail later.
                            return true;
                        }
                        else if (known_types_.size() > number_samples_.size())
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

    if (known_types_.size() < number_samples_.size())
    {
        std::cout << "ERROR: detected more than " << known_types_.size() << " publishers" << std::endl;
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
    void* sample_data = known_types_[reader->type().get_type_name()].obj_;

    if (reader->take_next_sample(sample_data, &info) == RETCODE_OK)
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {

            std::cout << "Subscriber " << reader->type().get_type_name() <<
                " received sample: " << info.sample_identity.sequence_number() << " -> ";

            // Call the callback function to process the received data
            known_types_[reader->type().get_type_name()].callback_(sample_data);

            if (max_number_samples_ <= ++number_samples_[info.sample_identity.writer_guid()])
            {
                cv_.notify_all();
            }
        }
    }
}