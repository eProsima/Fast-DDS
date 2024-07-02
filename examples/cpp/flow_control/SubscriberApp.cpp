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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace flow_control {

SubscriberApp::SubscriberApp(
        const CLIParser::flow_control_config& config)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new FlowControlPubSubType())
    , samples_(config.samples)
    , stop_(false)
    , slow_writer_guid({0})
    , fast_writer_guid({0})
{
    StatusMask status_mask = StatusMask::none();
    status_mask << StatusMask::data_available();
    status_mask << StatusMask::subscription_matched();
    // Create Participant
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT, this,
                    status_mask);
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    //Register the type
    type_.register_type(participant_);

    // Create Subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    // Retrieve default QoS, in case they have been previously set with an XML file
    participant_->get_default_subscriber_qos(sub_qos);
    subscriber_ = participant_->create_subscriber(sub_qos, nullptr, StatusMask::none());
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }

    // Create Topic
    topic_ = participant_->create_topic("flow_control_topic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create DataReader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    subscriber_->get_default_datareader_qos(reader_qos);
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
        DataReader*,
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
    FlowControl msg;
    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&msg, &info)))
    {
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            static unsigned int fastMessages = 0;
            static unsigned int slowMessages = 0;

            if (info.sample_identity.writer_guid().entityId == fast_writer_guid)
            {
                fastMessages++;
                std::cout << "Sample RECEIVED from FAST writer, index=" << msg.index() << std::endl;
            }
            else if (info.sample_identity.writer_guid().entityId == slow_writer_guid)
            {
                slowMessages++;
                std::cout << "Sample RECEIVED from SLOW writer, index=" << msg.index() << std::endl;
            }

            if ((samples_ > 0) && (fastMessages >= samples_) && (slowMessages >= samples_))
            {
                stop();
            }
        }
    }
}

void SubscriberApp::on_data_writer_discovery(
        DomainParticipant* /*participant*/,
        eprosima::fastdds::rtps::WriterDiscoveryInfo&& info,
        bool& /*should_be_ignored*/)
{
    std::vector<eprosima::fastdds::rtps::octet> slow_writer_id = {0};
    std::vector<eprosima::fastdds::rtps::octet> fast_writer_id = {1};

    if (info.info.m_qos.m_userData.data_vec() == fast_writer_id)
    {
        fast_writer_guid = info.info.guid().entityId;
        std::cout << "Fast writer id " << fast_writer_guid << std::endl;
    }
    else if (info.info.m_qos.m_userData.data_vec() == slow_writer_id)
    {
        slow_writer_guid = info.info.guid().entityId;
        std::cout << "Slow writer id " << slow_writer_guid << std::endl;
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

} // namespace flow_control
} // namespace examples
} // namespace fastdds
} // namespace eprosima
