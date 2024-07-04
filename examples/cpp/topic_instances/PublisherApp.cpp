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

#include <algorithm>
#include <condition_variable>
#include <csignal>
#include <stdexcept>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace topic_instances {

PublisherApp::PublisherApp(
        const CLIParser::publisher_config& config)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new ShapeTypePubSubType())
    , matched_(0)
    , period_ms_(config.interval)
    , timeout_ms_(config.timeout)
    , samples_(config.samples)
    , instances_(config.instances)
    , stop_(false)
    , shape_config_(config.shape_config)
{
    // Create the participant
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Shape_pub_participant");
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

    // Create the publisher
    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    participant_->get_default_publisher_qos(pub_qos);
    publisher_ = participant_->create_publisher(pub_qos, nullptr, StatusMask::none());
    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic(config.topic_name, type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the data writer
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    publisher_->get_default_datawriter_qos(writer_qos);
    writer_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    writer_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.history().kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    uint32_t sample_limit = samples_;
    if (sample_limit == 0)
    {
        sample_limit = DATAWRITER_QOS_DEFAULT.resource_limits().max_samples_per_instance;
    }
    writer_qos.resource_limits().max_instances = instances_;
    writer_qos.history().depth = sample_limit;
    writer_qos.resource_limits().max_samples_per_instance = sample_limit;
    writer_qos.resource_limits().max_samples = sample_limit * instances_;
    // Check if ASYNCHRONOUS publish mode is required
    if (instances_ > 1)
    {
        writer_qos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    }
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
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void PublisherApp::on_publication_matched(
        eprosima::fastdds::dds::DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher matched." << std::endl;
        cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
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
    while (!is_stopped() && ((samples_ == 0) || !instances_sent_all_samples()))
    {
        publish();
        // Wait for period or stop event
        std::unique_lock<std::mutex> terminate_lock(mutex_);
        cv_.wait_for(terminate_lock, std::chrono::milliseconds(period_ms_), [&]()
                {
                    return is_stopped();
                });
    }

    // Wait for the timeout to expire if not stopped
    if (!is_stopped() && timeout_ms_ > 0)
    {
        std::cout << "Publisher stopping... waiting for " << timeout_ms_ << " ms" << std::endl;
        std::unique_lock<std::mutex> timeout_lock(mutex_);
        cv_.wait_for(timeout_lock, std::chrono::milliseconds(timeout_ms_), [&]()
                {
                    return is_stopped();
                });
    }
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
        // Update shape values
        move(shape_config_.x, shape_config_.y, shape_config_.direction);
        for (int i = 0; i < instances_;  i++)
        {
            bool write_ret = false;
            // Prepare new shapes values
            std::string color = instances_ == 1 ? shape_config_.color : CLIParser::shape_color(i);
            shape_.color(color);
            shape_.shapesize(shape_config_.size);
            shape_.x(shape_config_.x);
            shape_.y(shape_config_.y);
            // Send shape
            write_ret = writer_->write(&shape_);

            // Print shape if successfully performed write call
            if (write_ret)
            {
                // Print sent shape data
                std::cout << shape_.color() << " Shape with size " << shape_.shapesize()
                          << " at X:" << shape_.x() << ", Y:" << shape_.y() << " SENT" << std::endl;
                // Add the sent sample to the corresponding instance counter
                if (samples_per_instance_.count(color) > 0)
                {
                    // Instance exists in the map, increase the counter
                    samples_per_instance_[color]++;
                }
                else
                {
                    // Instance does not exist in the map, set it with value 1
                    samples_per_instance_[color] = 1;
                }
            }
            ret = i == 0 ? write_ret : ret && write_ret;
        }
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

void PublisherApp::move(
        int& x,
        int& y,
        CLIParser::ShapeDirection& direction)
{
    switch (direction)
    {
        case CLIParser::ShapeDirection::UP:
        {
            y -= shape_config_.step;

            if (static_cast<uint64_t>(y) < shape_config_.lower_th && static_cast<uint64_t>(x) > shape_config_.lower_th + 1)
            {
                direction = CLIParser::ShapeDirection::RIGHT;
            }
            else if (static_cast<uint64_t>(y) < shape_config_.lower_th)
            {
                direction = CLIParser::ShapeDirection::DIAGONAL;
            }

            break;
        }
        case CLIParser::ShapeDirection::DOWN:
        {
            y += shape_config_.step;

            if (static_cast<uint64_t>(y) > shape_config_.vertical_th)
            {
                direction = CLIParser::ShapeDirection::LEFT;
            }

            break;
        }
        case CLIParser::ShapeDirection::LEFT:
        {
            x -= shape_config_.step;

            if (static_cast<uint64_t>(x) < shape_config_.lower_th)
            {
                direction = CLIParser::ShapeDirection::UP;
            }

            break;
        }
        case CLIParser::ShapeDirection::RIGHT:
        {
            x += shape_config_.step;

            if (static_cast<uint64_t>(x) > shape_config_.horizontal_th)
            {
                direction = CLIParser::ShapeDirection::DOWN;
            }

            break;
        }
        case CLIParser::ShapeDirection::DIAGONAL:
        {
            x += shape_config_.step;
            y += shape_config_.step;

            if (static_cast<uint64_t>(x) > (shape_config_.width / 2) && static_cast<uint64_t>(y) > (shape_config_.height / 2))
            {
                direction = CLIParser::ShapeDirection::UP;
            }

            break;
        }
    }
}

bool PublisherApp::instances_sent_all_samples()
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

} // namespace topic_instances
} // namespace examples
} // namespace fastdds
} // namespace eprosima
