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

#include "Application.hpp"
#include "CLIParser.hpp"
#include "ShapeTypePubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace topic_instances {

SubscriberApp::SubscriberApp(
        const CLIParser::subscriber_config& config)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new ShapeTypePubSubType())
    , samples_(config.samples)
    , stop_(false)
    , stop_receiving_samples_(false)
{
    // Create the participant
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Shape_sub_participant");

    // Set Shapes metadata
    pqos.properties().properties().emplace_back("fastdds.application.id", "SHAPES_DEMO", true);
    pqos.properties().properties().emplace_back("fastdds.application.metadata", "", true);
    participant_ = DomainParticipantFactory::get_instance()->create_participant(
        config.domain, pqos, nullptr, StatusMask::none());

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
    topic_ = participant_->create_topic(config.topic_name, type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the data reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    subscriber_->get_default_datareader_qos(reader_qos);
    reader_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    reader_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.history().kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    uint32_t sample_limit = samples_;
    if (samples_ == 0)
    {
        sample_limit = DATAREADER_QOS_DEFAULT.resource_limits().max_samples_per_instance;
    }
    reader_qos.resource_limits().max_instances = config.instances;
    reader_qos.history().depth = sample_limit;
    reader_qos.resource_limits().max_samples_per_instance = sample_limit;
    reader_qos.resource_limits().max_samples = sample_limit * config.instances;
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
    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&shape_, &info)))
    {
        if (!stop_receiving_samples_.load() && (info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            // Add the received sample to the corresponding instance handle counter
            if (samples_per_instance_.count(info.instance_handle) > 0)
            {
                // Instance handle exists in the map, increase the counter
                samples_per_instance_[info.instance_handle] += 1;
            }
            else
            {
                // Instance handle does not exist in the map, set it with value 1
                samples_per_instance_[info.instance_handle] = 1;
            }

            // Print the shape
            std::cout << shape_.color() << " Shape with size " << shape_.shapesize()
                      << " at X:" << shape_.x() << ", Y:" << shape_.y() << " RECEIVED" << std::endl;

            // Check if the execution should be stopped
            if ((samples_ > 0) && instances_received_all_samples())
            {
                stop_receiving_samples_.store(true);
                cv_.notify_all();
            }
        }
        else if (info.instance_state == NOT_ALIVE_DISPOSED_INSTANCE_STATE)
        {
            std::cout << shape_.color() << " Shape has been disposed" << std::endl;
            if (std::find(disposed_instances_.begin(), disposed_instances_.end(),
                    info.instance_handle) == disposed_instances_.end())
            {
                disposed_instances_.push_back(info.instance_handle);
            }
        }
        else if (info.instance_state == NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)
        {
            std::cout << shape_.color() << " Shape has been disposed by all publishers" << std::endl;
        }
    }
}

void SubscriberApp::run()
{
    {
        std::unique_lock<std::mutex> lock_(mutex_);
        cv_.wait(lock_, [&]
                {
                    return stop_receiving_samples_.load() || is_stopped();
                });
    }
    // Wait for period or stop event
    {
        std::unique_lock<std::mutex> timeout_lock(mutex_);
        cv_.wait_for(timeout_lock, std::chrono::milliseconds(50u), [&]()
                {
                    return instances_disposed() || is_stopped();
                });
    }
    stop();
}

bool SubscriberApp::is_stopped()
{
    return stop_.load();
}

void SubscriberApp::stop()
{
    stop_.store(true);
    cv_.notify_all();
}

bool SubscriberApp::instances_received_all_samples()
{
    bool ret = true;

    if (samples_per_instance_.size() > 0)
    {
        for (const auto& instance : samples_per_instance_)
        {
            if (instance.second < samples_)
            {
                ret = false;
                break;
            }
        }
    }
    else
    {
        ret = false;
    }
    return ret;
}

bool SubscriberApp::instances_disposed()
{
    bool ret = true;
    if (disposed_instances_.size() > 0)
    {
        for (const auto& instance : samples_per_instance_)
        {
            if (std::find(disposed_instances_.begin(), disposed_instances_.end(),
                    instance.first) == disposed_instances_.end())
            {
                ret = false;
                break;
            }
        }
    }
    else
    {
        ret = false;
    }
    return ret;
}

} // namespace topic_instances
} // namespace examples
} // namespace fastdds
} // namespace eprosima
