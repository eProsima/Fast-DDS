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
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include "CLIParser.hpp"
#include "HelloWorldPubSubTypes.hpp"
#include "Application.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace static_edp_discovery {

SubscriberApp::SubscriberApp(
        const CLIParser::static_edp_discovery_config& config,
        const std::string& topic_name)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType())
    , samples_(config.samples)
    , received_samples_(0)
    , stop_(false)
    , xml_file_name(config.xml_path)
{
    // Create the participant
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("static_edp_delivery_sub_participant");
    pqos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = false;
    pqos.wire_protocol().builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol = true;
    pqos.wire_protocol().builtin.discovery_config.static_edp_xml_config(xml_file_name.c_str());
    auto factory = DomainParticipantFactory::get_instance();
    if (RETCODE_OK != factory->check_xml_static_discovery(xml_file_name))
    {
        throw std::runtime_error("Error parsing subscriber xml file");
    }
    participant_ = factory->create_participant(config.domain, pqos, nullptr, StatusMask::none());
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Register the type
    type_.register_type(participant_);

    // Create the subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(sub_qos);
    subscriber_ = participant_->create_subscriber(sub_qos, nullptr, StatusMask::none());
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    uint32_t max_samples = samples_;
    if (max_samples == 0)
    {
        max_samples = DATAREADER_QOS_DEFAULT.resource_limits().max_samples_per_instance;
    }
    // Create the reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    subscriber_->get_default_datareader_qos(reader_qos);
    reader_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    reader_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.history().kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    reader_qos.history().depth = max_samples;
    reader_qos.resource_limits().max_samples_per_instance = max_samples;
    reader_qos.resource_limits().max_samples = reader_qos.resource_limits().max_instances * max_samples;
    reader_qos.endpoint().entity_id = 3;
    reader_qos.endpoint().user_defined_id = 4;
    reader_ = subscriber_->create_datareader(topic_, reader_qos, this, StatusMask::all());
    if (reader_ == nullptr)
    {
        throw std::runtime_error("DataReader initialization failed");
    }
}

SubscriberApp::~SubscriberApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
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
    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&hello_, &info)))
    {
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            received_samples_++;
            // Print Hello world message data
            std::cout << "Message: '" << hello_.message() << "' with index: '" << hello_.index()
                      << "' RECEIVED" << std::endl;
            if (samples_ > 0 && (received_samples_ >= samples_))
            {
                stop();
            }
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

} // namespace static_edp_discovery
} // namespace examples
} // namespace fastdds
} // namespace eprosima
