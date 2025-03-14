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
#include <fastdds/LibrarySettings.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

TypeLookupServicePublisher::~TypeLookupServicePublisher()
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

bool TypeLookupServicePublisher::init(
        uint32_t domain_id,
        std::vector<std::string> known_types,
        uint32_t builtin_flow_controller_bytes)
{
    domain_id_ = domain_id;
    create_type_creator_functions();

    LibrarySettings settings;
    settings.intraprocess_delivery = INTRAPROCESS_OFF;
    DomainParticipantFactory::get_instance()->set_library_settings(settings);

    auto qos = PARTICIPANT_QOS_DEFAULT;
    if (builtin_flow_controller_bytes > 0)
    {
        auto new_flow_controller = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
        new_flow_controller->name = "MyFlowController";
        new_flow_controller->max_bytes_per_period = builtin_flow_controller_bytes;
        new_flow_controller->period_ms = static_cast<uint64_t>(100000);
        qos.flow_controllers().push_back(new_flow_controller);
        qos.wire_protocol().builtin.flow_controller_name = new_flow_controller->name;
    }
    participant_ = DomainParticipantFactory::get_instance()
                    ->create_participant(domain_id, qos, this);
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

bool TypeLookupServicePublisher::setup_publisher(
        PubKnownType& a_type)
{
    std::string type_name = a_type.type_sup_.get_type_name();

    // CREATE THE PUBLISHER
    Publisher* publisher = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (publisher == nullptr)
    {
        std::cout << "ERROR TypeLookupServicePublisher: create_publisher: " << type_name << std::endl;
        return false;
    }

    // CREATE THE TOPIC
    std::ostringstream topic_name;
    topic_name << type_name << "_" << asio::ip::host_name() << "_" << domain_id_;
    Topic* topic = participant_->create_topic(topic_name.str(), a_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic == nullptr)
    {
        std::cout << "ERROR TypeLookupServicePublisher: create_topic: " << type_name << std::endl;
        return false;
    }

    // CREATE THE DATAWRITER
    DataWriterQos wqos = publisher->get_default_datawriter_qos();
    wqos.data_sharing().off();
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    a_type.writer_ = publisher->create_datawriter(topic, wqos);
    if (a_type.writer_ == nullptr)
    {
        std::cout << "ERROR TypeLookupServicePublisher: create_datawriter" << std::endl;
        return false;
    }
    return true;
}

bool TypeLookupServicePublisher::create_known_type(
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
    a_type.type_.reset(new Type());
    a_type.type_sup_.reset(new TypePubSubType());
    a_type.type_sup_.register_type(participant_);

    if (!setup_publisher(a_type))
    {
        return false;
    }

    std::lock_guard<std::mutex> guard(known_types_mutex_);
    known_types_.emplace(type, a_type);
    return true;
}

bool TypeLookupServicePublisher::create_discovered_type(
        const SubscriptionBuiltinTopicData& info)
{
    std::string new_type_name = info.type_name.to_string();
    // Check if the type is already created
    if (nullptr != participant_->find_type(new_type_name))
    {
        return false;
    }

    PubKnownType a_type;

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

    if (!setup_publisher(a_type))
    {
        return false;
    }

    std::lock_guard<std::mutex> guard(known_types_mutex_);
    known_types_.emplace(new_type_name, a_type);
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
        uint32_t expected_matches,
        uint32_t timeout)
{
    expected_matches_ = expected_matches;

    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_for(lock, std::chrono::seconds(timeout),
                    [&]()
                    {
                        return matched_ == static_cast<int32_t>(expected_matches_);
                    });

    if (!result)
    {
        std::cout << "ERROR TypeLookupServicePublisher discovery Timeout with matched = " <<
            matched_ << std::endl;
        return false;
    }
    return true;
}

bool TypeLookupServicePublisher::run(
        uint32_t samples,
        uint32_t timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_for(
        lock, std::chrono::seconds(timeout), [&]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            uint32_t current_sample {0};
            std::string type_name;
            while (samples > current_sample)
            {
                for (auto& known_type : known_types_)
                {
                    type_name = known_type.second.type_sup_.get_type_name();
                    if (known_type.second.dyn_type_)
                    {
                        DynamicData::_ref_type sample =
                        DynamicDataFactory::get_instance()->create_data(known_type.second.dyn_type_);
                        if (RETCODE_OK != known_type.second.writer_->write(&sample))
                        {
                            std::cout << "ERROR TypeLookupServicePublisher fails writing sample " <<
                                current_sample + 1 << std::endl;
                            return false;
                        }
                    }

                    if (known_type.second.type_)
                    {
                        void* sample = known_type.second.type_sup_.create_data();
                        if (RETCODE_OK != known_type.second.writer_->write(sample))
                        {
                            std::cout << "ERROR TypeLookupServicePublisher fails writing sample " <<
                                current_sample + 1 << std::endl;
                            return false;
                        }
                        known_type.second.type_sup_.delete_data(sample);
                    }

                    ++sent_samples_[known_type.second.writer_->guid()];
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                ++current_sample;
            }
            return true;
        });

    if (!result)
    {
        std::cout << "ERROR TypeLookupServicePublisher" << std::endl;
        if (expected_matches_ != sent_samples_.size())
        {
            std::cout << "Expected_matches_ = " << expected_matches_ <<
                " Working_writers_ = " << sent_samples_.size() << std::endl;
        }

        for (auto& sent_sample : sent_samples_)
        {
            if (samples != sent_sample.second)
            {
                std::cout << sent_sample.first << "Wrote: "
                          << sent_sample.second <<  " samples" << std::endl;
            }
        }
        return false;
    }
    return true;
}

void TypeLookupServicePublisher::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mutex_);
    matched_ += info.current_count_change;
    cv_.notify_all();
}

void TypeLookupServicePublisher::on_data_reader_discovery(
        DomainParticipant* /*participant*/,
        ReaderDiscoveryStatus reason,
        const SubscriptionBuiltinTopicData& info,
        bool& should_be_ignored)
{
    should_be_ignored = false;
    std::string discovered_reader_type_name = info.type_name.to_string();

    if (eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER == reason)
    {
        // Check if the type is already created
        if (nullptr == participant_->find_type(discovered_reader_type_name))
        {
            // Check for TypeObjectRegistry inconsistency
            std::string modified_type_name = discovered_reader_type_name;
            std::replace(modified_type_name.begin(), modified_type_name.end(), ':', '_');
            const bool has_type_object = types_without_typeobject_.find(modified_type_name) ==
                    types_without_typeobject_.end();
            const bool is_registered = check_registered_type(info.type_information);

            if ((has_type_object && !is_registered))
            {
                throw std::runtime_error("TypeLookupServiceSubscriber: Type '" +
                              discovered_reader_type_name + "' is not registered but it should be.");
            }
            if ((!has_type_object && is_registered))
            {
                throw std::runtime_error("TypeLookupServiceSubscriber: Type '" +
                              discovered_reader_type_name + "' is registered but it should not be.");
            }

            // Create new publisher for the type
            if (has_type_object)
            {
                create_types_threads.emplace_back(&TypeLookupServicePublisher::create_discovered_type, this, info);
            }
            else
            {
                create_types_threads.emplace_back(
                    &TypeLookupServicePublisher::create_known_type, this, discovered_reader_type_name);
            }
        }
    }
}
