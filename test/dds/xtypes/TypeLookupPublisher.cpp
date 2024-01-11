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

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

static int DOMAIN_ID_ = 10;

TypeLookupPublisher::~TypeLookupPublisher()
{
    for (auto it = known_types_.begin(); it != known_types_.end(); ++it)
    {
        it->second.publisher_->delete_datawriter(it->second.writer_);
        participant_->delete_publisher(it->second.publisher_);
        participant_->delete_topic(it->second.topic_);
    }

    if (nullptr != participant_)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool TypeLookupPublisher::init(
        std::vector<std::string> known_types)
{
    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(DOMAIN_ID_, PARTICIPANT_QOS_DEFAULT, this);
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
            std::cout << "ERROR TypeLookupPublisher::init uknown type: " << type << std::endl;
            return false;
        }
    }
    return true;
}

template <typename Type, typename TypePubSubType>
bool TypeLookupPublisher::create_known_type(
        const std::string& type)
{
    // Check if the type is already created
    if (known_types_.find(type) != known_types_.end())
    {
        std::cout << "Type " << type << " is already created." << std::endl;
        return true;
    }

    // Create a new PubKnownType for the given type
    PubKnownType a_type;
    a_type.obj_ = new Type();
    a_type.type_.reset(new TypePubSubType());
    a_type.type_.register_type(participant_);

    // CREATE THE PUBLISHER
    a_type.publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, this);
    if (a_type.publisher_ == nullptr)
    {
        std::cout << "ERROR create_publisher: " << type << std::endl;
        return false;
    }

    // CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << type << "_" << asio::ip::host_name() << "_" << DOMAIN_ID_;
    a_type.topic_ = participant_->create_topic(topic_name.str(), a_type.type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (a_type.topic_ == nullptr)
    {
        std::cout << "ERROR create_topic: " << type << std::endl;
        return false;
    }

    // CREATE THE DATAWRITER
    DataWriterQos wqos = a_type.publisher_->get_default_datawriter_qos();
    a_type.writer_ = a_type.publisher_->create_datawriter(a_type.topic_, wqos, this);
    if (a_type.writer_ == nullptr)
    {
        std::cout << "ERROR create_datawriter" << std::endl;
        return false;
    }

    // CREATE CALLBACK
    a_type.callback_ = [](void* data, int current_sample)
            {
                Type* typed_data = static_cast<Type*>(data);
                typed_data->index(current_sample);
                typed_data->message("Message" + std::to_string(current_sample));
            };

    known_types_.emplace(type, a_type);
    return true;
}

void TypeLookupPublisher::wait_discovery(
        uint32_t how_many)
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&]
            {
                return matched_ >= how_many;
            });
}

void TypeLookupPublisher::run(
        uint32_t samples)
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

            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        ++current_sample;
    }
}

void TypeLookupPublisher::on_publication_matched(
        DataWriter* /*publisher*/,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "(TypeLookupPublisher) matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "(TypeLookupPublisher) unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
    cv_.notify_all();
}