// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LargeDataPublisher.cpp
 *
 */

#include "LargeDataPublisher.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

#include "LargeDataPubSubTypes.h"

std::atomic<bool> LargeDataPublisher::running_(false);
std::mutex LargeDataPublisher::running_mtx_;
std::condition_variable LargeDataPublisher::running_cv_;

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

using IPLocator = eprosima::fastrtps::rtps::IPLocator;

LargeDataPublisher::LargeDataPublisher(const uint32_t data_size)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new LargeDataMsgPubSubType())
    , first_connected_(false)
    , frequency_(10)
    , removed_unacked_samples_(0)
{
    init_msg(data_size);
}

bool LargeDataPublisher::init(const std::string &tcp_type)
{
    //CREATE THE PARTICIPANT
    DomainParticipantQos pqos;

    const std::string WAN_IP = "127.0.0.1";
    const int PORT = 20000;

    if (tcp_type == "server")
    {
        // SERVER
        pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
        pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastrtps::Duration_t(5, 0);

        pqos.transport().use_builtin_transports = false;

        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();

        descriptor->sendBufferSize = 0;
        descriptor->receiveBufferSize = 0;

        descriptor->set_WAN_address(WAN_IP);
        descriptor->add_listener_port(PORT);

        pqos.transport().user_transports.push_back(descriptor);
    }

    else if (tcp_type == "client")
    {
        // CLIENT
        int32_t kind = LOCATOR_KIND_TCPv4;

        Locator initial_peer_locator;
        initial_peer_locator.kind = kind;

        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();

        IPLocator::setIPv4(initial_peer_locator, WAN_IP);
        initial_peer_locator.port = PORT;

        pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel

        pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
        pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastrtps::Duration_t(5, 0);

        pqos.transport().use_builtin_transports = false;
        pqos.transport().user_transports.push_back(descriptor);
    }

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (publisher_ == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    topic_ = participant_->create_topic("LargeDataTCPTopic",  type_.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    //CREATE THE DATAWRITER
    DataWriterQos wqos;

    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = VOLATILE_DURABILITY_QOS;
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 10;

    writer_ = publisher_->create_datawriter(topic_, wqos, this);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;
}

LargeDataPublisher::~LargeDataPublisher()
{
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }
    if (publisher_ != nullptr)
    {
        participant_->delete_publisher(publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void LargeDataPublisher::on_publication_matched(
        eprosima::fastdds::dds::DataWriter* /*writer*/,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        first_connected_ = true;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
    running_cv_.notify_all();
}

void LargeDataPublisher::on_unacknowledged_sample_removed(
        eprosima::fastdds::dds::DataWriter* /*writer*/,
        const eprosima::fastdds::dds::InstanceHandle_t& /*instance*/)
{
    std::cout << "Unacknowledged sample removed" << std::endl;
    removed_unacked_samples_++;
}

void LargeDataPublisher::run(
        uint16_t frequency)
{
    frequency_ = frequency;
    running_.store(true);
    signal(SIGINT, [](int /*signum*/)
            {
                std::cout << "SIGINT received, stopping Publisher execution." << std::endl;
                running_.store(false);
                running_cv_.notify_all();
            });

    std::cout << "Publisher running. Please press CTRL-C to stop the Publisher" << std::endl;

    std::thread working_thread(&LargeDataPublisher::publish, this);

    std::unique_lock<std::mutex> lck(running_mtx_);
    running_cv_.wait(lck, []
            {
                return !running_;
            });

    working_thread.join();

    std::cout << "Sent samples:            " << msg_.frameNumber() << std::endl;
    std::cout << "Unacked removed samples: " << removed_unacked_samples_ << std::endl;

}

void LargeDataPublisher::publish()
{
    // Wait for match
    std::unique_lock<std::mutex> lck(matched_mtx_);
    running_cv_.wait(lck, [this]
            {
                return !running_ || matched_ > 0;
            });

    std::cout << "Starting publication at " << frequency_ << " Hz" << std::endl;

    auto start_ = std::chrono::high_resolution_clock::now();

    while (running_ && matched_ > 0)
    {
        msg_.frameNumber(msg_.frameNumber() + 1);
        if (writer_->write(&msg_))
        {
            if (msg_.frameNumber() > 0 && msg_.frameNumber() % 2 == 0)
            {
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start_);

                std::cout << "Sending samples at " << ((double) 2 * 1000) / duration.count() << " Hz" << std::endl;

                start_ = std::chrono::high_resolution_clock::now();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / frequency_));
        }
        else
        {
            msg_.frameNumber(msg_.frameNumber() - 1);
            std::cout << "Something went wrong while sending frame " << msg_.frameNumber() << ". Closing down..." <<
                std::endl;
            running_.store(false);
            running_cv_.notify_all();
        }
    }
}

void LargeDataPublisher::init_msg(
    const uint32_t data_size)
{
    msg_.cameraId(1);
    msg_.frameNumber(0);
    msg_.frameTag(1);
    msg_.exposureDuration(0.5);
    msg_.gain(0.7);
    msg_.readoutDurationSeconds(0.01);
    msg_.captureTimestampNs(1);
    msg_.captureTimestampInProcessingClockDomainNs(1);
    msg_.arrivalTimestampNs(1);
    msg_.processingStartTimestampNs(1);
    msg_.temperatureDegC(35.5);
    ImageFormatMsg format;
    format.width(1);
    format.height(1);
    format.stride(1);
    format.format(PixelFormatEnum::FORMAT_1);
    msg_.imageFormat(format);
    msg_.videoCodecName("some_codec_name");
    msg_.imageBufferSize(50);
    msg_.data(std::vector<uint8_t>(data_size, 0xAA));
}
