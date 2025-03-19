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

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/LibrarySettings.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

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

bool TypeLookupServiceSubscriber::init(
        uint32_t domain_id,
        std::vector<std::string> known_types,
        uint32_t builtin_flow_controller_bytes)
{
    domain_id_ = domain_id;
    create_type_creator_functions();

    LibrarySettings settings;
    settings.intraprocess_delivery = INTRAPROCESS_OFF;
    DomainParticipantFactory::get_instance()->set_library_settings(settings);

    StatusMask mask = StatusMask::subscription_matched()
            << StatusMask::data_available()
            << StatusMask::liveliness_changed();

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
                    ->create_participant(domain_id, qos, this, mask);
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
    topic_name << type_name << "_" << asio::ip::host_name() << "_" << domain_id_;
    Topic* topic = participant_->create_topic(topic_name.str(), new_type.type_sup_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic == nullptr)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber: create_topic: " << type_name << std::endl;
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos = subscriber->get_default_datareader_qos();
    rqos.data_sharing().off();
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
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
    a_type.type_.reset(new Type());
    a_type.type_sup_.reset(new TypePubSubType());
    a_type.type_sup_.register_type(participant_);

    if (!setup_subscriber(a_type))
    {
        return false;
    }

    std::lock_guard<std::mutex> guard(known_types_mutex_);
    known_types_.emplace(type, a_type);
    return true;
}

template <typename Type>
bool TypeLookupServiceSubscriber::process_type_impl(
        DataReader* reader)
{
    eprosima::fastdds::dds::LoanableSequence<Type> datas;
    eprosima::fastdds::dds::SampleInfoSeq infos;

    ReturnCode_t success = reader->take(datas, infos);
    if (eprosima::fastdds::dds::RETCODE_OK != success)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber: error taking samples: " <<
            reader->type().get_type_name() << std::endl;
        return false;
    }

    for (int32_t i = 0; i < datas.length(); ++i)
    {
        Type& data = datas[i];
        eprosima::fastdds::dds::SampleInfo& info = infos[i];

        if (info.valid_data && reader->is_sample_valid(&data, &info))
        {
            std::lock_guard<std::mutex> guard(mutex_);
            received_samples_[info.sample_identity.writer_guid()]++;
            cv_.notify_all();
        }
        else
        {
            std::cout << "ERROR TypeLookupServiceSubscriber: sample invalid " <<
                reader->type().get_type_name() << std::endl;
            return false;
        }
    }
    reader->return_loan(datas, infos);
    return true;
}

bool TypeLookupServiceSubscriber::process_dyn_type_impl(
        DataReader* reader)
{
    eprosima::fastdds::dds::LoanableSequence<DynamicPubSubType> datas;
    eprosima::fastdds::dds::SampleInfoSeq infos;

    ReturnCode_t success = reader->take(datas, infos);
    if (eprosima::fastdds::dds::RETCODE_OK != success)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber: Error taking dynamic samples: " <<
            reader->type().get_type_name() << std::endl;
        return false;
    }

    for (int32_t i = 0; i < datas.length(); ++i)
    {
        DynamicPubSubType& data = datas[i];
        eprosima::fastdds::dds::SampleInfo& info = infos[i];

        if (info.valid_data && reader->is_sample_valid(&data, &info))
        {
            std::lock_guard<std::mutex> guard(mutex_);
            received_samples_[info.sample_identity.writer_guid()]++;
            cv_.notify_all();
        }
        else
        {
            std::cout << "ERROR TypeLookupServiceSubscriber: Dynamic sample invalid " <<
                reader->type().get_type_name() << std::endl;
            return false;
        }
    }

    if (RETCODE_OK != reader->return_loan(datas, infos))
    {
        return false;
    }
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
                        return matched_ == static_cast<int32_t>(expected_matches_);
                    });

    if (!result)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber discovery Timeout with matched = " <<
            matched_ << std::endl;
        return false;
    }
    return true;
}

bool TypeLookupServiceSubscriber::wait_participant_discovery(
        uint32_t expected_matches,
        uint32_t timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_for(lock, std::chrono::seconds(timeout),
                    [&]()
                    {
                        return participant_matched_ == static_cast<int32_t>(expected_matches);
                    });

    if (!result)
    {
        std::cout << "ERROR TypeLookupServiceSubscriber participoant discovery Timeout with matched = " <<
            participant_matched_ << std::endl;
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
        std::cout << "ERROR TypeLookupServiceSubscriber" << std::endl;
        if (expected_matches_ != received_samples_.size())
        {
            std::cout << "Expected_matches_ = " << expected_matches_ <<
                " Working_writers_ = " << received_samples_.size() << std::endl;
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
    matched_ += info.current_count_change;
    cv_.notify_all();
}

void TypeLookupServiceSubscriber::on_data_available(
        DataReader* reader)
{
    bool data_correct = false;
    auto& known_type = known_types_[reader->type().get_type_name()];

    if (known_type.dyn_type_)
    {
        data_correct = process_dyn_type_impl(reader);
    }
    if (known_type.type_)
    {
        // Find the type processor in the map
        auto it = type_processor_functions_.find(reader->type().get_type_name());
        if (it != type_processor_functions_.end())
        {
            // Call the associated type processor function
            data_correct = it->second(reader);
        }
        else
        {
            std::cout << "ERROR TypeLookupServiceSubscriber: Processed unknown type: " <<
                reader->type().get_type_name() << std::endl;
        }
    }

    if (!data_correct)
    {
        throw std::runtime_error("TypeLookupServiceSubscriber: Wrong data on_data_available: " +
                      reader->type().get_type_name());
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
            // Check for TypeObjectRegistry inconsistency
            std::string modified_type_name = discovered_writer_type_name;
            std::replace(modified_type_name.begin(), modified_type_name.end(), ':', '_');
            const bool has_type_object = types_without_typeobject_.find(modified_type_name) ==
                    types_without_typeobject_.end();
            const bool is_registered = check_registered_type(info.type_information);

            if ((has_type_object && !is_registered))
            {
                throw std::runtime_error("TypeLookupServiceSubscriber: Type '" +
                              discovered_writer_type_name + "' is not registered but it should be.");
            }
            if ((!has_type_object && is_registered))
            {
                throw std::runtime_error("TypeLookupServiceSubscriber: Type '" +
                              discovered_writer_type_name + "' is registered but it should not be.");
            }

            // Create new subscriber for the type
            if (has_type_object)
            {
                create_types_threads.emplace_back(&TypeLookupServiceSubscriber::create_discovered_type, this, info);
            }
            else
            {
                create_types_threads.emplace_back(
                    &TypeLookupServiceSubscriber::create_known_type, this, discovered_writer_type_name);
            }
        }
    }
}

void TypeLookupServiceSubscriber::on_participant_discovery(
        DomainParticipant* participant,
        eprosima::fastdds::rtps::ParticipantDiscoveryStatus status,
        const ParticipantBuiltinTopicData& info,
        bool& should_be_ignored)
{
    static_cast<void>(should_be_ignored);
    if (status == ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " discovered participant " << info.guid << ": " <<
            ++participant_matched_ << std::endl;
    }
    else if (status == ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " detected changes on participant " << info.guid
                  << std::endl;
    }
    else if (status == ParticipantDiscoveryStatus::REMOVED_PARTICIPANT ||
            status == ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " undiscovered participant " << info.guid << ": " <<
            --participant_matched_ << std::endl;
    }
    cv_.notify_all();
}
