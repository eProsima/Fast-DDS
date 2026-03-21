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

#include <stdexcept>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

#include "HelloWorldPubSubTypes.hpp"
#include "HelloWorldTypeObjectSupport.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace content_filter {
SubscriberApp::SubscriberApp(
        const CLIParser::subscriber_config& config,
        const std::string& topic_name)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType())
    , received_samples_(0)
    , samples_(config.samples)
    , filter_topic_(nullptr)
    , stop_(false)
{
    // Set DomainParticipant name
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");

    // Get DomainParticipantFactory instance
    auto factory = DomainParticipantFactory::get_instance();
    // Set intraprocess to OFF
    LibrarySettings library_settings;
    library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
    factory->set_library_settings(library_settings);
    pqos.transport().use_builtin_transports = false;
    std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_descriptor =
            std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    pqos.transport().user_transports.push_back(udp_descriptor);

    // Create DomainParticipant
    participant_ = factory->create_participant(1, pqos);
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // If using the custom filter
    if (config.filter_kind == CLIParser::FilterKind::CUSTOM)
    {
        // Register the filter factory
        if (eprosima::fastdds::dds::RETCODE_OK !=
                participant_->register_content_filter_factory("CUSTOM_FILTER", &filter_factory))
        {
            throw std::runtime_error("Custom filter initialization failed");
        }
    }

    // Register the type
    type_.register_type(participant_);

    // Create the Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }

    // Create the Topic
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the ContentFilteredTopic
    std::string expression;
    std::vector<std::string> parameters;
    if (config.filter_kind == CLIParser::FilterKind::CUSTOM)
    {
        // Custom filter: reject samples where index > parameters[0] and index < parameters[1].
        // Custom filter does not use expression. However, an empty expression disables filtering, so some expression
        // must be set.
        expression = " ";
        parameters.push_back(config.lower_bound);
        parameters.push_back(config.upper_bound);
        filter_topic_ =
                participant_->create_contentfilteredtopic("HelloWorldFilteredTopic1", topic_, expression, parameters,
                        "CUSTOM_FILTER");
    }
    else if (config.filter_kind == CLIParser::FilterKind::DEFAULT)
    {
        // Default filter: accept samples meeting the given expression: index between the two given parameters
        expression = config.filter_expression;
        parameters.push_back(config.lower_bound);
        parameters.push_back(config.upper_bound);
        filter_topic_ =
                participant_->create_contentfilteredtopic("HelloWorldFilteredTopic1", topic_, expression, parameters);
    }
    else if (config.filter_kind == CLIParser::FilterKind::NONE)
    {
        // An empty expression disables filtering
        expression = "";
        filter_topic_ =
                participant_->create_contentfilteredtopic("HelloWorldFilteredTopic1", topic_, expression, parameters);
    }

    if (filter_topic_ == nullptr)
    {
        throw std::runtime_error("Filter Topic initialization failed");
    }

    // Create the DataReader
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.data_sharing().off();
    // In order to receive all samples, DataReader is configured as RELIABLE and TRANSIENT_LOCAL (ensure reception even
    // if DataReader matching is after DataWriter one)
    if (config.reliable)
    {
        rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    }
    if (config.transient_local)
    {
        rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    }
    reader_ = subscriber_->create_datareader(filter_topic_, rqos, this);
    if (reader_ == nullptr)
    {
        throw std::runtime_error("Data Reader initialization failed");
    }
}

SubscriberApp::~SubscriberApp()
{
    if (participant_ != nullptr)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();
        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void SubscriberApp::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    // New remote DataWriter discovered
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched." << std::endl;
    }
    // New remote DataWriter undiscovered
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched." << std::endl;

        if (received_samples_ > 0 && (received_samples_ >= samples_))
        {
            stop();
        }
    }
    // Non-valid option
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
    // Take next sample from DataReader's history
    while ((!is_stopped()) && (eprosima::fastdds::dds::RETCODE_OK == reader->take_next_sample(&hello_, &info)))
    {
        // Some samples only update the instance state. Only if it is a valid sample (with data)
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            received_samples_++;
            // Print structure data
            std::cout << "Message: '" << hello_.message() << "' with index: '" << hello_.index()
                      << "' RECEIVED" << std::endl;
        }
    }
}

void SubscriberApp::run()
{
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, [&]
            {
                return is_stopped();
            });
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

} // namespace content_filter
} // namespace examples
} // namespace fastdds
} // namespace eprosima
