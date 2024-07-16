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

#include <thread>

#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

#include "HelloWorldPubSubTypes.hpp"
#include "HelloWorldTypeObjectSupport.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace content_filter {

PublisherApp::PublisherApp(
        const CLIParser::publisher_config& config,
        const std::string& topic_name)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new HelloWorldPubSubType())
    , matched_(0)
    , samples_(config.samples)
    , stop_(false)
    , period_ms_(config.interval)
    , i(0)
{
    // Initialize internal variables
    matched_ = 0;

    // Initialize data sample
    hello_.site("CDG");
    hello_.device("CAM1");

    // Set DomainParticipant name
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");

    // Get DomainParticipantFactory
    auto factory = DomainParticipantFactory::get_instance();

    // Create DomainParticipant in domain 0
    participant_ = factory->create_participant(1, pqos);
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Register the type
    type_.register_type(participant_);

    // Create the Publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the Topic
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the DataWriter
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    wqos.data_sharing().off();
    wqos.writer_resource_limits().reader_filters_allocation.initial = 0;
    wqos.writer_resource_limits().reader_filters_allocation.maximum = config.max_reader_filters;
    wqos.writer_resource_limits().reader_filters_allocation.increment = 1;
    if (!config.transient_local)
    {
        wqos.durability().kind = VOLATILE_DURABILITY_QOS;
    }
    if (!config.reliable)
    {
        wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }
    writer_ = publisher_->create_datawriter(topic_, wqos, this);
    if (writer_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }
}

PublisherApp::~PublisherApp()
{
    if (participant_ != nullptr)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();
        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void PublisherApp::on_publication_matched(
        DataWriter*,
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
    while (!is_stopped() && ((samples_ == 0) || (i < samples_)))
    {
        if (publish())
        {
            std::cout << "Message: '" << hello_.site() << "' with index: '" << hello_.device()
                      << "' SENT" << std::endl;
        }
        // Wait for period or stop event
        std::unique_lock<std::mutex> period_lock(mutex_);
        cv_.wait_for(period_lock, std::chrono::milliseconds(period_ms_), [&]()
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

    std::vector<std::string> site_ = {"CDG","SB8","HON","ABC","DEF","GHI","CDG","SB8","HON"};
    std::vector<std::string> device_ = {"CAM1","CAM2","CAM3","CAM1","CAM2","CAM3","CAM4","CAM5","CAM6"};
    i++;
    if (!is_stopped())
    {
        hello_.site(site_[(i)%9]);
        hello_.device(device_[(i)%9]);
        ret = writer_->write(&hello_);
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

} // namespace content_filter
} // namespace examples
} // namespace fastdds
} // namespace eprosima
