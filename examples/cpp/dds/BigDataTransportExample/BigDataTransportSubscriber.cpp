// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BigDataTransportSubscriber.cpp
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

#include "BigDataTransportSubscriber.h"

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
    , type_(new BigDataPubSubType())
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
        BuiltinTransports transport,
        bool reliable,
        bool transient,
        const std::string& partitions,
        bool use_ownership,
        const std::string& profile,
        uint32_t history)
{
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");

    if (profile.empty())
    {
        if (transport != BuiltinTransports::DEFAULT)
        {
            // If no transport set, environment variable is taken
            pqos.setup_transports(transport);
        }

        // CREATE THE PARTICIPANT
        participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);
    }
    else
    {
        // Create participant from xml profile
        participant_ = DomainParticipantFactory::get_instance()->create_participant_with_profile(profile);
    }

    if (participant_ == nullptr)
    {
        return false;
    }

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE SUBSCRIBER
    SubscriberQos sqos;

    if (!partitions.empty())
    {
        // Divide in partitions by ;
        std::stringstream spartitions(partitions);
        std::string partition_cut;
        while (std::getline(spartitions, partition_cut, ';'))
        {
            sqos.partition().push_back(partition_cut.c_str());
        }
    }

    subscriber_ = participant_->create_subscriber(sqos, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    // CREATE THE TOPIC
    topic_ = participant_->create_topic(topic_name, "BigData", TOPIC_QOS_DEFAULT);

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

    // Data sharing set in endpoint. If it is not default, set it to off
    if (transport != BuiltinTransports::DEFAULT)
    {
        rqos.data_sharing().off();
    }
    else
    {
        // rqos.data_sharing().automatic();  // default
    }

    if (reliable)
    {
        rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        rqos.history().kind = KEEP_ALL_HISTORY_QOS;
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

    if (history)
    {
        rqos.history().kind = KEEP_LAST_HISTORY_QOS;
        rqos.history().depth = history;
    }
    else
    {
        rqos.history().kind = KEEP_ALL_HISTORY_QOS;
    }

    // Set ownership
    if (use_ownership)
    {
        rqos.ownership().kind = OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
    }

    rqos.data_sharing().off();

    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    std::cout << "Subscriber Participant created with DataReader Guid [ " << reader_->guid() << " ]." << std::endl;

    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber()
{
    if (participant_ != nullptr)
    {
        if (subscriber_ != nullptr)
        {
            if (reader_ != nullptr)
            {
                subscriber_->delete_datareader(reader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void HelloWorldSubscriber::SubListener::set_max_messages(
        uint32_t max_messages)
{
    max_messages_ = max_messages;
}

size_t HelloWorldSubscriber::SubListener::get_size()
{
    return bigdata_.data().size();
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber matched [ " << iHandle2GUID(info.last_publication_handle) << " ]." << std::endl;
        time_match = std::chrono::high_resolution_clock::now();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber unmatched [ " << iHandle2GUID(info.last_publication_handle) << " ]." << std::endl;
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
    while ((reader->take_next_sample(&bigdata_, &info) == ReturnCode_t::RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            bytes_received_ += get_size();
            // Latency
            auto time_now = std::chrono::high_resolution_clock::now();
            auto time_when_sent =
                    std::chrono::seconds(bigdata_.timestamp_value().sec()) +
                    std::chrono::nanoseconds(bigdata_.timestamp_value().nanosec());

            auto elapsed_time = time_now - time_when_sent;

            // Duration in nanoseconds
            std::chrono::duration<double> elapsed = elapsed_time.time_since_epoch();
            count += 1;
            total_latency_ += elapsed.count();

            // Throughput
            auto running_time = time_now - time_match;

            double throughput = double(bytes_received_) / running_time.count() * double(1000);

            // std::cout <<
            // std::chrono::duration_cast<std::chrono::seconds>(
            //   time_now.time_since_epoch()).count() <<
            // '.' << std::setfill('0') << std::setw(9) <<
            // std::chrono::duration_cast<std::chrono::nanoseconds>(
            //   time_now.time_since_epoch()).count() % 1000000000 << std::endl;


            // std::cout <<
            //   elapsed_time.time_since_epoch().count() <<
            //   ',' << transmission_size <<
            //   ',' << std::fixed << rate <<
            //   ',' << std::fixed << bandwidth_bps <<
            //   ',' << std::1000000000fixed << bandwidth_mbps <<
            //   std::endl;

            // Messages Lost
            uint32_t lost_messages = 0;
            lost_messages = bigdata_.index() - last_index_ - 1;
            last_index_ = bigdata_.index();
            // if (!first_received_)
            // {
            //     last_index_ = bigdata_.index();
            //     first_received_ = true;
            //     lost_messages = bigdata_.index() - last_index_;
            // }
            // else
            // {
            //     lost_messages = bigdata_.index() - last_index_ - 1;
            //     last_index_ = bigdata_.index();
            // }

            if (lost_messages > 0)
            {
                total_lost_ += lost_messages;
            }

            std::cout << "[" << bigdata_.index() << "]: TP (MB/s): " << std::fixed <<
                std::setw(5) << std::setprecision(3) << throughput << ", Latency: " <<
                std::fixed << std::setw(8) << std::setprecision(4) << total_latency_ / count <<
                ", ML: " << total_lost_ << ", Time: " << running_time.count() / double(1000000000) << std::endl;
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void HelloWorldSubscriber::SubListener::on_sample_lost(
        DataReader* reader,
        const SampleLostStatus& status)
{
    std::cout << "Total samples lost: " << status.total_count << std::endl;
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
