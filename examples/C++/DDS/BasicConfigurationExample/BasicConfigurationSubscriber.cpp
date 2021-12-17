// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BasicConfigurationSubscriber.cpp
 *
 */

#include <csignal>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include "BasicConfigurationSubscriber.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> HelloWorldSubscriber::stop_(false);
std::mutex HelloWorldSubscriber::terminate_cv_mtx_;
std::condition_variable HelloWorldSubscriber::terminate_cv_;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldSubscriber::is_stopped()
{
    return stop_;
}

void HelloWorldSubscriber::stop()
{
    stop_ = true;
    terminate_cv_.notify_all();
}

bool HelloWorldSubscriber::init(
        const std::string& topic_name,
        uint32_t max_messages,
        uint32_t domain,
        TransportType transport,
        bool reliable,
        bool transient)
{
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");

    // TRANSPORT CONFIG
    // If it is set, not use default and set the transport
    if (transport != DEFAULT)
    {
        pqos.transport().use_builtin_transports = false;

        if (transport == SHM || transport == DATA_SHARING)
        {
            auto shm_transport = std::make_shared<SharedMemTransportDescriptor>();
            pqos.transport().user_transports.push_back(shm_transport);
        }
        else if (transport == UDP)
        {
            auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
            pqos.transport().user_transports.push_back(udp_transport);
        }
        else if (transport == UDPv6)
        {
            auto udp_transport = std::make_shared<UDPv6TransportDescriptor>();
            pqos.transport().user_transports.push_back(udp_transport);
        }
    }

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    // CREATE THE TOPIC
    topic_ = participant_->create_topic(
        topic_name,
        "HelloWorld",
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE READER
    if (max_messages > 0)
    {
        listener_.set_max_messages(max_messages);
    }
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;

    // Data sharing set in endpoint. If it is not default or datasharing, set it to off
    if (transport != DEFAULT && transport != DATA_SHARING)
    {
        rqos.data_sharing().off();
    }
    else
    {
        rqos.data_sharing().automatic();  // default
    }

    if (reliable)
    {
        rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        rqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;  // default
    }

    if (transient)
    {
        rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    }
    else
    {
        rqos.durability().kind = VOLATILE_DURABILITY_QOS;   // default
    }

    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    if (participant_ != nullptr)
    {
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            if (reader_ != nullptr)
            {
                subscriber_->delete_datareader(reader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void HelloWorldSubscriber::SubListener::set_max_messages(
        uint32_t max_messages)
{
    max_messages_ = max_messages;
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    while ((reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message " << hello_.message().data() << " " << hello_.index() << " RECEIVED" << std::endl;
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void HelloWorldSubscriber::run(
        uint32_t samples)
{
    stop_ = false;
    if (samples > 0)
    {
        std::cout << "Subscriber running until " << samples <<
            " samples have been received. Please press CTRL+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Subscriber running. Please press CTRL+C to stop the Subscriber." << std::endl;
    }
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Subscriber execution." << std::endl;
                static_cast<void>(signum); HelloWorldSubscriber::stop();
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
