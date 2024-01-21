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
 * @file TypeLookupPublisher.cpp
 */

#include "TypeLookupPublisher.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>


using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

static int PUB_DOMAIN_ID_ = 10;
static int PUB_MAX_TIMEOUT_ = 10;

TypeLookupPublisher::~TypeLookupPublisher()
{
    for (auto it = known_types_.begin(); it != known_types_.end(); ++it)
    {
        if (nullptr != it->second.writer_)
        {
            it->second.publisher_->delete_datawriter(it->second.writer_);
        }
        if (nullptr != it->second.publisher_)
        {
            participant_->delete_publisher(it->second.publisher_);
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

bool TypeLookupPublisher::init(
        std::vector<std::string> known_types)
{
    auto settings = fastrtps::xmlparser::XMLProfileManager::library_settings();
    settings.intraprocess_delivery = fastrtps::INTRAPROCESS_OFF;
    fastrtps::xmlparser::XMLProfileManager::library_settings(settings);

    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(PUB_DOMAIN_ID_, PARTICIPANT_QOS_DEFAULT, this);
    if (participant_ == nullptr)
    {
        std::cout << "ERROR TypeLookupSubscriber: create_participant" << std::endl;
        return false;
    }

    REGISTER_TYPE(Type1);
    REGISTER_TYPE(Type2);
    REGISTER_TYPE(Type3);
    REGISTER_TYPE(TypeBig);
    REGISTER_TYPE(TypeDep);

    for (const auto& type : known_types)
    {
        create_known_type(type);
    }
    return true;
}

bool TypeLookupPublisher::create_known_type(
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
bool TypeLookupPublisher::create_known_type_impl(
        const std::string& type)
{
    // Create a new PubKnownType for the given type
    PubKnownType a_type;
    a_type.obj_ = new Type();
    a_type.type_.reset(new TypePubSubType());
    a_type.type_.register_type(participant_);


    // CREATE THE PUBLISHER
    a_type.publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, this);
    if (a_type.publisher_ == nullptr)
    {
        std::cout << "ERROR TypeLookupSubscriber: create_publisher: " << type << std::endl;
        return false;
    }

    // CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << type << "_" << asio::ip::host_name() << "_" << PUB_DOMAIN_ID_;
    a_type.topic_ = participant_->create_topic(topic_name.str(), a_type.type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (a_type.topic_ == nullptr)
    {
        std::cout << "ERROR TypeLookupSubscriber: create_topic: " << type << std::endl;
        return false;
    }

    // CREATE THE DATAWRITER
    DataWriterQos wqos = a_type.publisher_->get_default_datawriter_qos();
    wqos.liveliness().lease_duration = 3;
    wqos.liveliness().announcement_period = 1;
    wqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;

    a_type.writer_ = a_type.publisher_->create_datawriter(a_type.topic_, wqos, this);
    if (a_type.writer_ == nullptr)
    {
        std::cout << "ERROR TypeLookupSubscriber: create_datawriter" << std::endl;
        return false;
    }

    // CREATE CALLBACK
    a_type.callback_ = [type](void* data, int current_sample)
            {
                Type* typed_data = static_cast<Type*>(data);
                //typed_data->index(current_sample);
            };

    known_types_.emplace(type, a_type);
    return true;
}

bool TypeLookupPublisher::check_registered_type(
        const xtypes::TypeInformationParameter& type_info)
{
    xtypes::TypeObject type_obj;
    return RETCODE_OK == DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
        type_info.type_information.complete().typeid_with_size().type_id(), type_obj);
}

bool TypeLookupPublisher::wait_discovery(
        uint32_t expected_match,
        uint32_t timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_for(lock, std::chrono::seconds(timeout), [&]()
                    {
                        return matched_ == expected_match;
                    });
    return result;
}

bool TypeLookupPublisher::run(
        uint32_t samples)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(
        lock, std::chrono::seconds(PUB_MAX_TIMEOUT_), [&]
        {
            uint32_t current_sample = 0;
            void* sample = nullptr;

            while (samples > current_sample)
            {
                for (auto it = known_types_.begin(); it != known_types_.end(); ++it)
                {
                    sample =  it->second.type_.create_data();

                    it->second.callback_(sample, current_sample);

                    std::cout << "Publisher " << it->second.type_.get_type_name() <<
                        " writting sample: " << current_sample << std::endl;
                    it->second.writer_->write(sample);
                    it->second.type_.delete_data(sample);

                    ++sent_samples_[it->second.writer_->guid()];

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                ++current_sample;
            }
            return true;
        });
}

void TypeLookupPublisher::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mutex_);
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

void TypeLookupPublisher::on_data_reader_discovery(
        eprosima::fastdds::dds::DomainParticipant* /*participant*/,
        eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info)
{
    xtypes::TypeInformationParameter type_info = info.info.type_information();

    if (check_registered_type(type_info))
    {
        create_known_type(info.info.typeName().to_string());
    }
    else
    {
        throw TypeLookupPublisherTypeNotRegisteredException(info.info.typeName().to_string());
    }
}