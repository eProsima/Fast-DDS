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
 * @file LargeDataSubscriber.cpp
 *
 */

#include "LargeDataSubscriber.h"

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <chrono>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/InstanceState.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/types/TypesBase.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

#include "LargeDataPubSubTypes.h"

std::atomic<bool> LargeDataSubscriber::running_(false);
std::mutex LargeDataSubscriber::mtx_;
std::condition_variable LargeDataSubscriber::cv_;

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

using IPLocator = eprosima::fastrtps::rtps::IPLocator;

LargeDataSubscriber::LargeDataSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new LargeDataMsgPubSubType())
    , matched_(0)
    , received_samples_(0)
    , lost_samples_(0)
{
}

bool LargeDataSubscriber::init(const std::string &tcp_type)
{
    // CREATE THE PARTICIPANT
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

    //CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    topic_ = participant_->create_topic("LargeDataTCPTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
    //CREATE THE DATAREADER
    DataReaderQos rqos;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.durability().kind = VOLATILE_DURABILITY_QOS;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 10;

    reader_ = subscriber_->create_datareader(topic_, rqos, this);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

LargeDataSubscriber::~LargeDataSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void LargeDataSubscriber::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber matched" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber unmatched" << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void LargeDataSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (reader->take_next_sample(&msg_, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (received_samples_ == 0)
        {
            start_ = std::chrono::high_resolution_clock::now();
        }

        else if (received_samples_ % 2 == 0)
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start_);

            std::cout << "Receiving samples at " << ((double) 2 * 1000) / duration.count() << " Hz" << std::endl;

            start_ = std::chrono::high_resolution_clock::now();
        }

        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            received_samples_++;
        }
    }
}

void LargeDataSubscriber::on_sample_lost(
        eprosima::fastdds::dds::DataReader* /*reader*/,
        const eprosima::fastdds::dds::SampleLostStatus& status)
{
    lost_samples_ = status.total_count;
}

void LargeDataSubscriber::run()
{
    running_.store(true);
    signal(SIGINT, [](int /*signum*/)
            {
                std::cout << "SIGINT received, stopping Subscriber execution." << std::endl;
                running_.store(false);
                cv_.notify_all();
            });

    std::cout << "Subscriber running. Please press CTRL-C to stop the Subscriber" << std::endl;

    std::unique_lock<std::mutex> lck(mtx_);
    cv_.wait(lck, []
            {
                return !running_;
            });

    std::cout << "Received samples: " << received_samples_ << std::endl;
    std::cout << "Lost samples:     " << lost_samples_ << std::endl;
}
