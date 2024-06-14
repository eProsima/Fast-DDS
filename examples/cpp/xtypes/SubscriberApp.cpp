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
 * @file SubscriberApp.cpp
 *
 */

#include "SubscriberApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include "CLIParser.hpp"
#include "Application.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace xtypes {

SubscriberApp::SubscriberApp(
        const CLIParser::config& config,
        const std::string& topic_name)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_name_(topic_name)
    , topic_(nullptr)
    , reader_(nullptr)
    , samples_(config.samples)
    , received_samples_(0)
    , stop_(false)
{
    // Create the participant
    auto factory = DomainParticipantFactory::get_instance();

    StatusMask participant_mask = StatusMask::data_available();
    participant_mask << eprosima::fastdds::dds::StatusMask::subscription_matched();

    participant_ = factory->create_participant_with_default_profile(this, participant_mask);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }
}

SubscriberApp::~SubscriberApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        if (RETCODE_OK != participant_->delete_contained_entities())
        {
            // C++11 dtors default to noexcept
            std::cerr << "Error deleting contained entities" << std::endl;
        }

        // Delete DomainParticipant
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->delete_participant(participant_))
        {
            // C++11 dtors default to noexcept
            std::cerr << "Error deleting participant" << std::endl;
        }
    }
}

void SubscriberApp::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void SubscriberApp::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    while (RETCODE_OK == reader->take_next_sample(&hello_, &info))
    {
        if (ALIVE_INSTANCE_STATE == info.instance_state && info.valid_data &&
                TK_STRUCTURE == remote_type_object_.complete()._d())
        {
            // Increase received samples
            received_samples_++;

            std::cout << "Message received:" << std::endl;

            // Get all the members by name
            DynamicTypeMembersByName members;

            if (RETCODE_OK != hello_->type()->get_all_members_by_name(members))
            {
                throw std::runtime_error("Error getting members by name");
            }

            // Print all the members
            for (auto elem : members)
            {
                std::cout << "  - " << elem.first << ": ";

                MemberDescriptor::_ref_type member_descriptor = {traits<MemberDescriptor>::make_shared()};

                if (RETCODE_OK != elem.second->get_descriptor(member_descriptor))
                {
                    throw std::runtime_error("Error getting member descriptor for member");
                }

                switch (member_descriptor->type()->get_kind())
                {
                    case TK_UINT32:
                    {
                        uint32_t uint_data {0};

                        if (RETCODE_OK != hello_->get_uint32_value(uint_data, elem.second->get_id()))
                        {
                            throw std::runtime_error("Error getting uint32 value");
                        }

                        std::cout << uint_data << std::endl;
                        break;
                    }
                    case TK_STRING8:
                    {
                        std::string str_data {0};

                        if (RETCODE_OK != hello_->get_string_value(str_data, elem.second->get_id()))
                        {
                            throw std::runtime_error("Error getting string value");
                        }

                        std::cout << "'" << str_data << "'" << std::endl;
                        break;
                    }
                    default:
                    {
                        std::cout << "Unhandled type " << member_descriptor->type()->get_kind() << std::endl;
                        break;
                    }
                }
            }
        }

        // Stop if the sample limit has been reached
        if (samples_ > 0 && (received_samples_ >= samples_))
        {
            stop();
        }
    }
}

void SubscriberApp::on_data_writer_discovery(
        DomainParticipant* /*participant*/,
        eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info,
        bool& should_be_ignored)
{
    // We don't want to ignore the writer
    should_be_ignored = false;

    // Check if the discovered topic is the one we are interested in
    if (topic_name_ == info.info.topicName().to_string())
    {
        // Get remote type information and use it to retrieve the type object
        auto type_info = info.info.type_information().type_information;
        auto type_id = type_info.complete().typeid_with_size().type_id();

        if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    type_id,
                    remote_type_object_))
        {
            std::cout << "Cannot get discovered type from registry:" << std::endl;
            std::cout << "  - Topic name: " << info.info.topicName() << std::endl;
            std::cout << "  - Type name:  " << info.info.typeName() << std::endl;
        }

        // Notify run thread that type has been discovered
        type_discovered_.store(true);
        terminate_cv_.notify_one();
    }
}

void SubscriberApp::run()
{
    // Wait for type discovery
    {
        std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
        terminate_cv_.wait(lck, [&]
                {
                    return is_stopped() || type_discovered_.load();
                });
    }

    // Create entities unless we need to exit
    if (type_discovered_)
    {
        initialize_entities();
    }

    // Wait for shutdown command
    {
        std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
        terminate_cv_.wait(lck, [&]
                {
                    return is_stopped();
                });
    }
}

bool SubscriberApp::is_stopped()
{
    return stop_.load();
}

void SubscriberApp::stop()
{
    stop_.store(true);
    terminate_cv_.notify_all();
}

void SubscriberApp::initialize_entities()
{
    // Register remotely discovered type
    DynamicTypeBuilder::_ref_type type_builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
        remote_type_object_);

    if (!type_builder)
    {
        throw std::runtime_error("Error creating type builder");
    }

    // Build the type
    remote_type_ = type_builder->build();

    if (!remote_type_)
    {
        throw std::runtime_error("Error building type");
    }

    TypeSupport dyn_type_support(new DynamicPubSubType(remote_type_));

    if (RETCODE_OK != dyn_type_support.register_type(participant_))
    {
        throw std::runtime_error("Error registering type");
    }

    // Initialize the DynamicData
    hello_ = DynamicDataFactory::get_instance()->create_data(remote_type_);

    if (!hello_)
    {
        throw std::runtime_error("Error creating DynamicData");
    }

    // Create the subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(sub_qos);
    subscriber_ = participant_->create_subscriber(sub_qos);

    if (nullptr == subscriber_)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic(topic_name_, dyn_type_support.get_type_name(), topic_qos);

    if (nullptr == topic_)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    subscriber_->get_default_datareader_qos(reader_qos);
    reader_ = subscriber_->create_datareader(topic_, reader_qos);

    if (nullptr == reader_)
    {
        throw std::runtime_error("DataReader initialization failed");
    }
}

} // namespace xtypes
} // namespace examples
} // namespace fastdds
} // namespace eprosima
