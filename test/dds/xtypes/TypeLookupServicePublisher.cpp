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
 * @file TypeLookupServicePublisher.cpp
 */

#include "TypeLookupServicePublisher.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>


using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

static int PUB_DOMAIN_ID_ = 10;

TypeLookupServicePublisher::~TypeLookupServicePublisher()
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

    for (auto& thread : create_known_types_threads)
    {
        thread.join();
    }
}

void TypeLookupServicePublisher::create_type_creator_functions()
{
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type1);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type2);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type3);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type3);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type4);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type5);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type6);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type7);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type8);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type9);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type10);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type11);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type12);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type13);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type14);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type15);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type16);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type17);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type18);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type19);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type20);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type21);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type22);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type23);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type24);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type25);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type26);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type27);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type28);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type29);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type30);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type31);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type32);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type33);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type34);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type35);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type36);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type37);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type38);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type39);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type40);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type41);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type42);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type43);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type44);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type45);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type46);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type47);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type48);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type49);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type50);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type51);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type52);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type53);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type54);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type55);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type56);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type57);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type58);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type59);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type60);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type61);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type62);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type63);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type64);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type65);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type66);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type67);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type68);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type69);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type70);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type71);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type72);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type73);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type74);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type75);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type76);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type77);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type78);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type79);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type80);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type81);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type82);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type83);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type84);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type85);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type86);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type87);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type88);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type89);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type90);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type91);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type92);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type93);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type94);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type95);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type96);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type97);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type98);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type99);
    PUBLISHER_TYPE_CREATOR_FUNCTION(Type100);
    PUBLISHER_TYPE_CREATOR_FUNCTION(TypeBig);
    PUBLISHER_TYPE_CREATOR_FUNCTION(TypeDep);
    PUBLISHER_TYPE_CREATOR_FUNCTION(TypeNoTypeObject);
}

bool TypeLookupServicePublisher::init(
        std::vector<std::string> known_types)
{
    create_type_creator_functions();

    auto settings = fastrtps::xmlparser::XMLProfileManager::library_settings();
    settings.intraprocess_delivery = fastrtps::INTRAPROCESS_OFF;
    fastrtps::xmlparser::XMLProfileManager::library_settings(settings);

    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(PUB_DOMAIN_ID_, PARTICIPANT_QOS_DEFAULT, this);
    if (participant_ == nullptr)
    {
        std::cout << "ERROR TypeLookupServicePublisher: create_participant" << std::endl;
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

bool TypeLookupServicePublisher::create_known_type(
        const std::string& type)
{
    // Find the type creator in the map
    auto it = type_creator_functions_.find(type);
    if (it != type_creator_functions_.end())
    {
        // Call the associated type creator function
        return it->second(type);
    }
    else
    {
        std::cout << "ERROR TypeLookupServicePublisher: init unknown type: " << type << std::endl;
        return false;
    }
}

template <typename Type, typename TypePubSubType>
bool TypeLookupServicePublisher::create_known_type_impl(
        const std::string& type)
{
    // Create a new PubKnownType for the given type
    PubKnownType a_type;
    a_type.type_.reset(new TypePubSubType());
    a_type.type_.register_type(participant_);


    // CREATE THE PUBLISHER
    a_type.publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, this);
    if (a_type.publisher_ == nullptr)
    {
        std::cout << "ERROR TypeLookupServicePublisher: create_publisher: " << type << std::endl;
        return false;
    }

    // CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << type << "_" << asio::ip::host_name() << "_" << PUB_DOMAIN_ID_;
    a_type.topic_ =
            participant_->create_topic(topic_name.str(), a_type.type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (a_type.topic_ == nullptr)
    {
        std::cout << "ERROR TypeLookupServicePublisher: create_topic: " << type << std::endl;
        return false;
    }

    // CREATE THE DATAWRITER
    DataWriterQos wqos = a_type.publisher_->get_default_datawriter_qos();
    wqos.liveliness().lease_duration = 3;
    wqos.liveliness().announcement_period = 1;
    wqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
    wqos.data_sharing().off();

    a_type.writer_ = a_type.publisher_->create_datawriter(a_type.topic_, wqos, this);
    if (a_type.writer_ == nullptr)
    {
        std::cout << "ERROR TypeLookupServicePublisher: create_datawriter" << std::endl;
        return false;
    }

    {
        std::lock_guard<std::mutex> guard(known_types_mutex_);
        known_types_.emplace(type, a_type);
    }

    return true;
}

bool TypeLookupServicePublisher::check_registered_type(
        const xtypes::TypeInformationParameter& type_info)
{
    xtypes::TypeObject type_obj;
    return RETCODE_OK == DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
        type_info.type_information.complete().typeid_with_size().type_id(), type_obj);
}

bool TypeLookupServicePublisher::wait_discovery(
        uint32_t expected_discoveries,
        uint32_t timeout)
{
    expected_matches_ = expected_discoveries;

    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_for(lock, std::chrono::seconds(timeout),
                    [&]()
                    {
                        return matched_ == expected_matches_;
                    });

    if (!result)
    {
        std::cout << "ERROR TypeLookupServicePublisher discovery Timeout with matched = " <<
            matched_ << std::endl;
        return false;
    }
    return true;
}

void TypeLookupServicePublisher::notify_discovery(
        std::string type_name,
        xtypes::TypeInformationParameter type_info)
{
    if (unique_types_.insert(type_name).second)
    {
        const bool should_be_registered = type_name.find("NoTypeObject") == std::string::npos;
        if (should_be_registered && !check_registered_type(type_info))
        {
            throw TypeLookupServicePublisherTypeRegistryException(type_name +
                          (should_be_registered ? " registered" : " not registered"));
        }
        std::unique_lock<std::mutex> lock(mutex_);
        ++matched_;
        cv_.notify_one();
    }
}

bool TypeLookupServicePublisher::run_for(
        uint32_t timeout)
{
    std::this_thread::sleep_for(std::chrono::seconds(timeout));
    return true;
}

void TypeLookupServicePublisher::on_data_reader_discovery(
        DomainParticipant* /*participant*/,
        fastrtps::rtps::ReaderDiscoveryInfo&& info)
{
    create_known_types_threads.emplace_back(&TypeLookupServicePublisher::notify_discovery,
            this, info.info.typeName().to_string(), info.info.type_information());
}
