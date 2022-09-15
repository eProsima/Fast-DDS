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
#include <fastdds/dds/log/Log.hpp>

#include "BasicConfigurationSubscriber.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<uint32_t> HelloWorldSubscriber::n_topics_(0);
std::atomic<bool> HelloWorldSubscriber::stop_(false);
std::atomic<uint32_t> HelloWorldSubscriber::n_stopped_(0);
std::mutex HelloWorldSubscriber::terminate_cv_mtx_;
std::condition_variable HelloWorldSubscriber::terminate_cv_;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldSubscriber::is_stopped()
{
    return stop_;
}

void HelloWorldSubscriber::stop(
        bool force)
{
    n_stopped_++;
    if (force || (n_stopped_.load() == n_topics_.load()))
    {
        stop_ = true;
        terminate_cv_.notify_all();
    }
}

bool HelloWorldSubscriber::init(
        std::vector<std::string> topic_names,
        uint32_t max_messages,
        uint32_t domain,
        TransportType transport,
        bool reliable,
        bool transient,
        bool realloc,
        bool dynamic)
{
    n_topics_.store(topic_names.size());

    DomainParticipantQos pqos;
    pqos.name("Participant_sub");

    // TRANSPORT CONFIG
    // If it is set, not use default and set the transport
    if (transport != DEFAULT)
    {
        pqos.transport().use_builtin_transports = false;

        if (transport == SHM)
        {
            auto shm_transport = std::make_shared<SharedMemTransportDescriptor>();
            pqos.transport().user_transports.push_back(shm_transport);
        }
        else if (transport == UDPv4)
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

    // CONFIGURE READER QOS
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;

    // Data sharing set in endpoint. If it is not default, set it to off
    if (transport != DEFAULT)
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

    if (realloc)
    {
        rqos.endpoint().history_memory_policy =
                eprosima::fastrtps::rtps::MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }
    else if (dynamic)
    {
        rqos.endpoint().history_memory_policy =
                eprosima::fastrtps::rtps::MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE;
    }

    for (uint32_t i = 0; i < n_topics_.load(); i++)
    {
        std::string topic_name = topic_names[i];
        std::shared_ptr<SubListener> listener = std::make_shared<SubListener>();
        listener->topic_name_ = topic_name;

        // CREATE THE TOPIC
        eprosima::fastdds::dds::Topic* topic = participant_->create_topic(
            topic_name,
            "HelloWorld",
            TOPIC_QOS_DEFAULT);

        if (topic == nullptr)
        {
            return false;
        }

        topics_.push_back(topic);

        // CREATE THE READER
        if (max_messages > 0)
        {
            listener->set_max_messages(max_messages);
        }

        listeners_.push_back(listener);

        eprosima::fastdds::dds::DataReader* reader = subscriber_->create_datareader(topic, rqos, listener.get());

        if (reader == nullptr)
        {
            return false;
        }

        readers_.push_back(reader);
    }

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    if (participant_ != nullptr)
    {
        for (uint32_t i = 0; i < n_topics_.load(); i++)
        {
            if (topics_[i] != nullptr)
            {
                participant_->delete_topic(topics_[i]);
            }
        }
        if (subscriber_ != nullptr)
        {
            for (uint32_t i = 0; i < n_topics_.load(); i++)
            {
                if (readers_[i] != nullptr)
                {
                    subscriber_->delete_datareader(readers_[i]);
                }
            }
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void SubListener::set_max_messages(
        uint32_t max_messages)
{
    max_messages_ = max_messages;
}

void SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << Log::get_timestamp() << " | " << "Subscriber matched in " << topic_name_ << std::endl;

    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << Log::get_timestamp() << " | " << "Subscriber unmatched in " << topic_name_ << std::endl;

    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    while ((reader->take_next_sample(&hello_,
            &info) == ReturnCode_t::RETCODE_OK) && !HelloWorldSubscriber::is_stopped())
    {
        samples_++;
        // Print your structure data here.
        std::cout << Log::get_timestamp() << " | " << "Message " <<
                hello_.index() << " RECEIVED in " << topic_name_ << std::endl;
        if (max_messages_ > 0 && samples_ == max_messages_)
        {
            HelloWorldSubscriber::stop();
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
                static_cast<void>(signum); HelloWorldSubscriber::stop(true);
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}
