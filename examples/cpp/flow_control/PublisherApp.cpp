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

#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace flow_control {

PublisherApp::PublisherApp(
        const CLIParser::flow_control_config& config)
    : participant_(nullptr)
    , fast_publisher_(nullptr)
    , slow_publisher_(nullptr)
    , topic_(nullptr)
    , fast_writer_(nullptr)
    , slow_writer_(nullptr)
    , type_(new FlowControlPubSubType())
    , matched_(0)
    , samples_(config.samples)
    , stop_(false)
{
    // Create Participant
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastdds::c_TimeInfinite;
    pqos.name("Participant_publisher");

    // This controller allows 300kb per second.
    auto slow_flow_controller_descriptor = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
    slow_flow_controller_descriptor->name = "slow_flow_controller_descriptor";
    slow_flow_controller_descriptor->max_bytes_per_period = 300000;
    slow_flow_controller_descriptor->period_ms = static_cast<uint64_t>(1000);
    pqos.flow_controllers().push_back(slow_flow_controller_descriptor);

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    //Register the type
    type_.register_type(participant_);

    // Create fast Publisher, which has no controller of its own.
    fast_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr, StatusMask::none());
    if (fast_publisher_ == nullptr)
    {
        throw std::runtime_error("Fast Publisher initialization failed");
    }

    // Create Topic
    topic_ = participant_->create_topic("flow_control_topic", type_.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create fast DataWriter
    DataWriterQos wfqos = DATAWRITER_QOS_DEFAULT;
    wfqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;

    fast_writer_ = fast_publisher_->create_datawriter(topic_, wfqos, this, StatusMask::all());
    if (fast_writer_ == nullptr)
    {
        throw std::runtime_error("Fast DataWriter initialization failed");
    }
    std::cout << "Fast publisher created, waiting for Subscribers." << std::endl;

    // Create slow Publisher, with its own controller
    slow_publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr, StatusMask::none());
    if (slow_publisher_ == nullptr)
    {
        throw std::runtime_error("Slow Publisher initialization failed");
    }

    // Create slow DataWriter
    DataWriterQos wsqos = DATAWRITER_QOS_DEFAULT;
    wsqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    wsqos.publish_mode().flow_controller_name = slow_flow_controller_descriptor->name;

    slow_writer_ = slow_publisher_->create_datawriter(topic_, wsqos, this, StatusMask::all());
    if (slow_writer_ == nullptr)
    {
        throw std::runtime_error("Slow DataWriter initialization failed");
    }
    std::cout << "Slow publisher created, waiting for Subscribers." << std::endl;
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
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
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
    // Publication code
    FlowControl st;

    /* Initialize your structure here */
    int msgsent_fast = 0;
    int msgsent_slow = 0;
    char ch;
    std::cout << "Flow Control example." << std::endl;
    std::cout << "Press \"f\" to send a sample through the fast writer, which has unlimited bandwidth" << std::endl;
    std::cout <<
        "Press \"s\" to send a sample through the slow writer, which is also limited by its own Flow Controller" <<
        std::endl;
    std::cout << "Press \"q\" quit" << std::endl;
    while (std::cin >> ch)
    {
        if (ch == 'f')
        {
            st.wasFast(true);
            // Wait for the data endpoints discovery
            std::unique_lock<std::mutex> matched_lock(mutex_);
            cv_.wait(matched_lock, [&]()
                    {
                        // at least one has been discovered
                        return ((matched_ > 0) || is_stopped());
                    });

            if (!is_stopped())
            {
                fast_writer_->write(&st);
            }
            ++msgsent_fast;
            std::cout << "Sending sample, count=" << msgsent_fast <<
                " through the fast writer. Send another sample? (f-fast,s-slow,q-quit): ";
        }
        else if (ch == 's')
        {
            st.wasFast(false);
            slow_writer_->write(&st);
            // Wait for the data endpoints discovery
            std::unique_lock<std::mutex> matched_lock(mutex_);
            cv_.wait(matched_lock, [&]()
                    {
                        // at least one has been discovered
                        return ((matched_ > 0) || is_stopped());
                    });

            if (!is_stopped())
            {
                fast_writer_->write(&st);
            }
            ++msgsent_slow;
            std::cout << "Sending sample, count=" << msgsent_slow <<
                " through the slow writer. Send another sample? (f-fast,s-slow,q-quit): ";
        }
        else if (ch == 'q')
        {
            std::cout << "Finishing Flow Control example" << std::endl;
            stop();
            break;
        }
        else
        {
            std::cout << "Command " << ch << " not recognized, please enter \"f/s/q\":";
        }

    }
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

} // namespace flow_control
} // namespace examples
} // namespace fastdds
} // namespace eprosima
