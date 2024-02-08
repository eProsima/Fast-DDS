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
 * @file TypeLookupServiceSubscriber.cpp
 *
 */

#include "TypeLookupServiceSubscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/LibrarySettings.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

static int SUB_DOMAIN_ID_ = 10;

TypeLookupServiceSubscriber::~TypeLookupServiceSubscriber()
{
    if (nullptr != participant_)
    {
        participant_->delete_contained_entities();
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
        participant_ = nullptr;
    }

    for (auto& thread : create_types_threads)
    {
        thread.join();
    }
}

void TypeLookupServiceSubscriber::create_type_creator_functions()
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
    SUBSCRIBER_TYPE_CREATOR_FUNCTION(TypeNoTypeObject);

}

bool TypeLookupServiceSubscriber::init(
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
        std::cout << "ERROR TypeLookupServiceSubscriber: create_participant" << std::endl;
        return false;
    }

    for (const auto& type : known_types)
    {
        if (!create_known_type(type))
        {
            return false;
        }
    }

    return true;
}

bool TypeLookupServiceSubscriber::setup_subscriber(
        SubKnownType& new_type)
{
    std::string type_name = new_type.type_sup_.get_type_name();

    //CREATE THE SUBSCRIBER
    Subscriber* subscriber = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber == nullptr)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber: create_subscriber: " << type_name << std::endl;
        return false;
    }

    //CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << type_name << "_" << asio::ip::host_name() << "_" << SUB_DOMAIN_ID_;
    Topic* topic = participant_->create_topic(topic_name.str(), new_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic == nullptr)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber: create_topic: " << type_name << std::endl;
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos = subscriber->get_default_datareader_qos();
    rqos.data_sharing().off();
    DataReader* reader = subscriber->create_datareader(topic, rqos);
    if (reader == nullptr)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber: create_datareader: " << type_name << std::endl;
        return false;
    }
    return true;
}

bool TypeLookupServiceSubscriber::create_known_type(
        const std::string& type)
{
    // Check if the type is already created
    if (nullptr != participant_->find_type(type))
    {
        return false;
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
        std::cout << "ERROR TypeLookupServiceSubscriber: init unknown type: " << type << std::endl;
        return false;
    }
}

template <typename Type, typename TypePubSubType>
bool TypeLookupServiceSubscriber::create_known_type_impl(
        const std::string& type)
{
    // Create a new SubKnownType for the given type
    SubKnownType a_type;
    a_type.type_ = new Type();
    a_type.type_sup_.reset(new TypePubSubType());
    a_type.type_sup_.register_type(participant_);

    // CREATE PRINT METHOD
    a_type.print_values_ = [](void* sample)
            {
                Type* typed_data = static_cast<Type*>(sample);
                std::cout <<  "content(" << typed_data->content() << ")" << std::endl;
            };

    if (!setup_subscriber(a_type))
    {
        return false;
    }

    std::lock_guard<std::mutex> guard(known_types_mutex_);
    known_types_.emplace(type, a_type);
    return true;
}

bool TypeLookupServiceSubscriber::create_discovered_type(
        const PublicationBuiltinTopicData& info)
{
    std::string new_type_name = info.type_name.to_string();
    // Check if the type is already created
    if (nullptr != participant_->find_type(new_type_name))
    {
        return false;
    }

    SubKnownType a_type;

    //CREATE THE DYNAMIC TYPE
    xtypes::TypeObject type_object;
    if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                info.type_information.type_information.complete().typeid_with_size().type_id(), type_object))

    {
        std::cout << "ERROR: TypeObject cannot be retrieved for type: " << new_type_name << std::endl;
        return false;
    }

    // Create DynamicType
    a_type.dyn_type_ = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(type_object)->build();
    if (!a_type.dyn_type_)
    {
        std::cout << "ERROR: DynamicType cannot be created for type: " << new_type_name << std::endl;
        return false;
    }

    // Register the data type
    a_type.type_sup_.reset(new DynamicPubSubType(a_type.dyn_type_));
    if (RETCODE_OK != a_type.type_sup_.register_type(participant_))
    {
        std::cout << "ERROR: DynamicType cannot be registered for type: " << new_type_name << std::endl;
        return false;
    }

    // CREATE PRINT METHOD
    a_type.dyn_print_values_ = [](DynamicData::_ref_type sample)
            {
                std::string content;
                sample->get_string_value(content, 0);
                std::cout <<  "content(" << content << ")" << std::endl;
            };

    if (!setup_subscriber(a_type))
    {
        return false;
    }

    std::lock_guard<std::mutex> guard(known_types_mutex_);
    known_types_.emplace(new_type_name, a_type);
    return true;
}

bool TypeLookupServiceSubscriber::check_registered_type(
        const xtypes::TypeInformationParameter& type_info)
{
    xtypes::TypeObject type_obj;
    return RETCODE_OK == DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
        type_info.type_information.complete().typeid_with_size().type_id(), type_obj);
}

bool TypeLookupServiceSubscriber::wait_discovery(
        uint32_t expected_matches,
        uint32_t timeout)
{
    expected_matches_ = expected_matches;

    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_for(lock, std::chrono::seconds(timeout),
                    [&]()
                    {
                        return matched_ == expected_matches_;
                    });

    if (!result)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber discovery Timeout with matched = " <<
            matched_ << std::endl;
        return false;
    }
    return true;
}

bool TypeLookupServiceSubscriber::run(
        uint32_t samples,
        uint32_t timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool result =  cv_.wait_for(
        lock, std::chrono::seconds(timeout), [&]
        {
            if (expected_matches_ != received_samples_.size())
            {
                return false;
            }

            for (auto& received_sample : received_samples_)
            {
                if (samples != received_sample.second)

                {
                    return false;
                }
            }
            return true;
        });

    if (!result)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber run Timeout" << std::endl;

        if (expected_matches_ != received_samples_.size())
        {
            std::cout << "expected_matches_ = " << expected_matches_ <<
                " matched_ = " << received_samples_.size() << std::endl;
        }
        for (auto& received_sample : received_samples_)
        {
            if (samples != received_sample.second)
            {
                std::cout << "From: " << received_sample.first <<
                    " samples: " << received_sample.second << "/" << samples << std::endl;
            }
        }

        return false;
    }
    return true;
}

void TypeLookupServiceSubscriber::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (info.current_count_change == 1)
    {
        ++matched_;
        cv_.notify_all();
    }
}

void TypeLookupServiceSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    auto& known_type = known_types_[reader->type().get_type_name()];

    if (known_type.dyn_type_)
    {
        DynamicData::_ref_type sample = DynamicDataFactory::get_instance()->create_data(known_type.dyn_type_);
        if (reader->take_next_sample(&sample, &info) == RETCODE_OK && info.valid_data)
        {
            std::cout << "Subscriber dyn_type_" << reader->type().get_type_name() << ": ";
            known_type.dyn_print_values_(sample);
            received_samples_[info.sample_identity.writer_guid()]++;
            cv_.notify_all();
        }
    }

    if (known_type.type_)
    {
        void* sample = known_type.type_;
        if (reader->take_next_sample(sample, &info) == RETCODE_OK && info.valid_data)
        {
            std::cout << "Subscriber type_" << reader->type().get_type_name() << ": ";
            known_type.print_values_(sample);
            received_samples_[info.sample_identity.writer_guid()]++;
            cv_.notify_all();
        }
    }
}

void TypeLookupServiceSubscriber::on_data_writer_discovery(
        DomainParticipant* /*participant*/,
        WriterDiscoveryStatus reason,
        const PublicationBuiltinTopicData& info,
        bool& should_be_ignored)
{
    should_be_ignored = false;
    std::string discovered_writer_type_name = info.type_name.to_string();

    if (eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER == reason)
    {
        // Check if the type is already created
        if (participant_->find_type(discovered_writer_type_name) == nullptr)
        {
            // Check type registration
            const bool should_be_registered = discovered_writer_type_name.find("NoTypeObject") == std::string::npos;

            if ((should_be_registered && !check_registered_type(info.type_information)) ||
                    (!should_be_registered && check_registered_type(info.type_information)))
            {
                throw std::runtime_error(discovered_writer_type_name +
                              (should_be_registered ? " registered" : " not registered"));
            }

            // Create new subscriber for the type
            if (should_be_registered)
            {
                create_types_threads.emplace_back(&TypeLookupServiceSubscriber::create_discovered_type, this, info);
            }
        }
    }
}
