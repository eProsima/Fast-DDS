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
 * @file PublisherApp.cpp
 *
 */

#include "PublisherApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace xtypes {

PublisherApp::PublisherApp(
        const CLIParser::config& config,
        const std::string& topic_name)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , matched_(0)
    , samples_(config.samples)
    , stop_(false)
{
    // Create the participant
    auto factory = DomainParticipantFactory::get_instance();
    participant_ = factory->create_participant_with_default_profile(nullptr, StatusMask::none());

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Create the type
    DynamicType::_ref_type dynamic_type = create_type(config.use_xml);
    if (!dynamic_type)
    {
        throw std::runtime_error("Error creating dynamic type");
    }

    // Set up the data type with initial values
    hello_ = DynamicDataFactory::get_instance()->create_data(dynamic_type);

    if (!hello_)
    {
        throw std::runtime_error("Error creating dynamic data");
    }

    hello_->set_uint32_value(hello_->get_member_id_by_name("index"), 0);
    hello_->set_string_value(hello_->get_member_id_by_name("message"), "Hello xtypes world");

    // Register the type
    TypeSupport type(new DynamicPubSubType(dynamic_type));

    if (RETCODE_OK != type.register_type(participant_))
    {
        throw std::runtime_error("Type registration failed");
    }

    // Create the publisher
    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    participant_->get_default_publisher_qos(pub_qos);
    publisher_ = participant_->create_publisher(pub_qos);

    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic(topic_name, type.get_type_name(), topic_qos);

    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the data writer
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    publisher_->get_default_datawriter_qos(writer_qos);
    writer_ = publisher_->create_datawriter(topic_, writer_qos, this, StatusMask::all());

    if (writer_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }
}

PublisherApp::~PublisherApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        if (RETCODE_OK != participant_->delete_contained_entities())
        {
            // C++11 dtors default to noexcept
            std::cerr << "Error deleting the contained entities." << std::endl;
        }

        // Delete DomainParticipant
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->delete_participant(participant_))
        {
            // C++11 dtors default to noexcept
            std::cerr << "Error deleting the participant." << std::endl;
        }
    }
}

void PublisherApp::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher matched." << std::endl;
        cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void PublisherApp::run()
{
    uint32_t index = get_uint32_value(hello_, "index");
    std::string message = get_string_value(hello_, "message");

    while (!is_stopped() && ((samples_ == 0) || (index < samples_)))
    {
        if (publish())
        {
            // Retrieve the new index and message
            index = get_uint32_value(hello_, "index");
            message = get_string_value(hello_, "message");

            std::cout << "Message sent:" << std::endl;
            std::cout << "  - index: " << index << std::endl;
            std::cout << "  - message: '" << message << "'" << std::endl;
        }
        else if (!is_stopped())
        {
            index = get_uint32_value(hello_, "index");
            std::cout << "Error sending message with index" << index << std::endl;
        }

        // Wait for period or stop event
        std::unique_lock<std::mutex> period_lock(mutex_);
        cv_.wait_for(period_lock, std::chrono::milliseconds(period_ms_), [&]()
                {
                    return is_stopped();
                });
    }

    // Wait for acknowledgments with 500 ms timeout
    writer_->wait_for_acknowledgments({0, 500000000});
}

bool PublisherApp::publish()
{
    bool ret = false;
    // Wait for the data endpoints discovery
    std::unique_lock<std::mutex> matched_lock(mutex_);
    cv_.wait(matched_lock, [&]()
            {
                // at least one has been discovered
                return ((matched_ > 0) || is_stopped());
            });

    if (!is_stopped())
    {
        // Increase the index by 1
        uint32_t index = get_uint32_value(hello_, "index");
        set_uint32_value(hello_, "index", index);

        // Publish the sample
        ret = (RETCODE_OK == writer_->write(&hello_));
    }
    return ret;
}

bool PublisherApp::is_stopped()
{
    return stop_.load();
}

void PublisherApp::stop()
{
    stop_.store(true);
    cv_.notify_one();
}

DynamicType::_ref_type PublisherApp::create_type(
        bool use_xml_type)
{
    DynamicTypeBuilder::_ref_type struct_builder;
    if (use_xml_type)
    {
        // Retrieve the type builder from xml
        if (RETCODE_OK !=
                DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("HelloWorld",
                struct_builder))
        {
            std::cout <<
                "Error getting dynamic type \"HelloWorld\"." << std::endl;
            return nullptr;
        }
    }
    else
    {
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        type_descriptor->kind(TK_STRUCTURE);
        type_descriptor->name("HelloWorld");
        struct_builder = DynamicTypeBuilderFactory::get_instance()->create_type(type_descriptor);

        if (!struct_builder)
        {
            throw std::runtime_error("Error creating type builder");
        }

        // Add index member
        MemberDescriptor::_ref_type index_member_descriptor {traits<MemberDescriptor>::make_shared()};
        index_member_descriptor->name("index");
        index_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));

        if (RETCODE_OK != struct_builder->add_member(index_member_descriptor))
        {
            throw std::runtime_error("Error adding index member");
        }

        // Add message member
        MemberDescriptor::_ref_type message_member_descriptor {traits<MemberDescriptor>::make_shared()};
        message_member_descriptor->name("message");
        message_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->create_string_type(static_cast<
                    uint32_t>(
                    LENGTH_UNLIMITED))->build());

        if (!message_member_descriptor)
        {
            throw std::runtime_error("Error creating string type");
        }

        if (RETCODE_OK != struct_builder->add_member(message_member_descriptor))
        {
            throw std::runtime_error("Error adding message member");
        }
    }

    // Build the type
    return struct_builder->build();
}

uint32_t PublisherApp::get_uint32_value(
        const DynamicData::_ref_type data,
        const std::string& member_name)
{
    uint32_t ui32 {0};

    if (RETCODE_OK != data->get_uint32_value(ui32, data->get_member_id_by_name(member_name)))
    {
        auto error_msg = "Error getting " + member_name + " value";
        throw std::runtime_error(error_msg);
    }

    return ui32;
}

void PublisherApp::set_uint32_value(
        DynamicData::_ref_type data,
        const std::string& member_name,
        const uint32_t value)
{
    if (RETCODE_OK != data->set_uint32_value(data->get_member_id_by_name(member_name), value + 1))
    {
        auto error_msg = "Error setting " + member_name + " value";
        throw std::runtime_error(error_msg);
    }
}

std::string PublisherApp::get_string_value(
        const DynamicData::_ref_type data,
        const std::string& member_name)
{
    std::string str;

    if (RETCODE_OK != data->get_string_value(str, data->get_member_id_by_name(member_name)))
    {
        auto error_msg = "Error getting " + member_name + " value";
        throw std::runtime_error(error_msg);
    }

    return str;
}

} // namespace xtypes
} // namespace examples
} // namespace fastdds
} // namespace eprosima
