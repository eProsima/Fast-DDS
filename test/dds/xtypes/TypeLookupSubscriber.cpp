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

static int SUB_DOMAIN_ID_ = 10;

TypeLookupSubscriber::~TypeLookupSubscriber()
{
    for (auto it = known_types_.begin(); it != known_types_.end(); ++it)
    {
        if (nullptr != it->second.reader_)
        {
            it->second.subscriber_->delete_datareader(it->second.reader_);
        }
        if (nullptr != it->second.subscriber_)
        {
            participant_->delete_subscriber(it->second.subscriber_);
        }
        if (nullptr != it->second.topic_)
        {
            participant_->delete_topic(it->second.topic_);
        }
    }

    if (nullptr != participant_)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void TypeLookupSubscriber::create_type_creator_functions()
{
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type1);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type2);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type3);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type3);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type4);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type5);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type6);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type7);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type8);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type9);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type10);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type11);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type12);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type13);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type14);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type15);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type16);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type17);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type18);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type19);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type20);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type21);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type22);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type23);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type24);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type25);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type26);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type27);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type28);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type29);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type30);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type31);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type32);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type33);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type34);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type35);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type36);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type37);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type38);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type39);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type40);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type41);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type42);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type43);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type44);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type45);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type46);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type47);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type48);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type49);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type50);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type51);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type52);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type53);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type54);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type55);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type56);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type57);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type58);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type59);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type60);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type61);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type62);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type63);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type64);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type65);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type66);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type67);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type68);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type69);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type70);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type71);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type72);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type73);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type74);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type75);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type76);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type77);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type78);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type79);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type80);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type81);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type82);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type83);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type84);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type85);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type86);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type87);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type88);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type89);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type90);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type91);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type92);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type93);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type94);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type95);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type96);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type97);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type98);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type99);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type100);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(TypeBig);
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(TypeDep);
}

bool TypeLookupSubscriber::init(
        std::vector<std::string> known_types)
{
    create_type_creator_functions();

    LibrarySettings settings;
    settings.intraprocess_delivery = INTRAPROCESS_OFF;
    DomainParticipantFactory::get_instance()->set_library_settings(settings);

    StatusMask mask = StatusMask::subscription_matched()
            << StatusMask::data_available()
            << StatusMask::liveliness_changed();

    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(SUB_DOMAIN_ID_, PARTICIPANT_QOS_DEFAULT, this, mask);
    if (participant_ == nullptr)
    {
        std::cout << "ERROR TypeLookupSubscriber: create_participant" << std::endl;
        return false;
    }

    for (const auto& type : known_types)
    {
        create_known_type(type);
    }

    return true;
}

bool TypeLookupSubscriber::create_known_type(
        const std::string& type)
{
    // Check if the type is already created
    if (nullptr != participant_->find_type(type))
    {
        return true;
    }

    // Find the type creator in the map
    auto it = type_creator_functions_.find(type);
    if (it != type_creator_functions_.end())
    {
        // Call the associated type creator function
        return it->second(type);
    }
    else
    {
        std::cout << "ERROR TypeLookupSubscriber: init unknown type: " << type << std::endl;
        return false;
    }
}

template <typename Type, typename TypePubSubType>
bool TypeLookupSubscriber::create_known_type_impl(
        const std::string& type)
{
    // Create a new PubKnownType for the given type
    SubKnownType a_type;
    a_type.obj_ = new Type();
    a_type.type_.reset(new TypePubSubType());
    a_type.type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    a_type.subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (a_type.subscriber_ == nullptr)
    {
        std::cout << "ERROR TypeLookupSubscriber: create_subscriber" << std::endl;
        return false;
    }

    //CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << type << "_" << asio::ip::host_name() << "_" << SUB_DOMAIN_ID_;
    a_type.topic_ = participant_->create_topic(topic_name.str(), a_type.type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (a_type.topic_ == nullptr)
    {
        std::cout << "ERROR TypeLookupSubscriber: create_topic" << std::endl;
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos = a_type.subscriber_->get_default_datareader_qos();
    a_type.reader_ = a_type.subscriber_->create_datareader(a_type.topic_, rqos);
    if (a_type.reader_ == nullptr)
    {
        std::cout << "ERROR TypeLookupSubscriber: create_datareader" << std::endl;
        return false;
    }

    // CREATE CALLBACK
    a_type.callback_ = [](void* data)
            {
                Type* sample = static_cast<Type*>(data);
                std::cout <<  "index(" << sample->index() << ")" << std::endl;
            };

    known_types_.emplace(type, a_type);

    return true;
}

bool TypeLookupSubscriber::check_registered_type(
        const xtypes::TypeInformationParameter& type_info)
{
    xtypes::TypeObject type_obj;
    return RETCODE_OK == DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
        type_info.type_information.complete().typeid_with_size().type_id(), type_obj);
}

bool TypeLookupSubscriber::wait_discovery(
        uint32_t expected_match,
        uint32_t timeout)
{
    std::unique_lock<std::mutex> lock(matched_mutex_);
    bool result = cv_.wait_for(lock, std::chrono::seconds(timeout),
                    [&]()
                    {
                        std::cout <<  "TypeLookupSubscriber::wait_discovery(" <<
                            matched_ << "/" << expected_match << ")" << std::endl;
                        return matched_ == expected_match;
                    });

    return result;
}

bool TypeLookupSubscriber::run(
        uint32_t samples,
        uint32_t timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(
        lock, std::chrono::seconds(timeout), [&]
        {
            if (known_types_.size() != received_samples_.size())
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(received_samples_mutex_);
            for (auto& received_sample : received_samples_)
            {
                if (samples != received_sample.second)
                {
                    return false;
                }
            }
            return true;
        });
}

void TypeLookupSubscriber::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(matched_mutex_);
    if (info.current_count_change == 1)
    {
        ++matched_;
    }
    else if (info.current_count_change == -1)
    {
        --matched_;
    }
    else
    {
        std::cout << "ERROR TypeLookupSubscriber: info.current_count_change" << std::endl;
    }
    cv_.notify_all();
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
                " received sample:" << info.sample_identity.sequence_number() << "->";
            // Call the callback function to process the received data
            known_types_[reader->type().get_type_name()].callback_(sample_data);

            received_samples_[info.sample_identity.writer_guid()]++;
            cv_.notify_all();
        }
    }
}

void TypeLookupSubscriber::on_data_writer_discovery(
        eprosima::fastdds::dds::DomainParticipant* /*participant*/,
        eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info,
        bool& should_be_ignored)
{
    should_be_ignored = false;
    xtypes::TypeInformationParameter type_info = info.info.type_information();

    if (check_registered_type(type_info))
    {
        create_known_type(info.info.typeName().to_string());
    }
    else
    {
        throw TypeLookupSubscriberTypeNotRegisteredException(info.info.typeName().to_string());
    }
}
