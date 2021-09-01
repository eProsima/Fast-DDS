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
 * @file HelloWorldSubscriber.cpp
 *
 */

#include "HelloWorldSubscriber.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>

#include <condition_variable>
#include <mutex>
#include <csignal>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

namespace sub_ns {
bool stop;
std::mutex mtx;
std::condition_variable terminate_cv;
} // namespace sub_ns

using namespace sub_ns;

HelloWorldSubscriber::HelloWorldSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldSubscriber::init(
        const std::string& topic_name,
        uint32_t threshold,
        uint32_t domain,
        const std::string& transport,
        bool reliable,
        bool transient)
{
    DomainParticipantQos pqos;
    pqos.name("Participant_sub");
    if (!transport.empty())
    {
        pqos.transport().use_builtin_transports = false;
        if (transport == "shm")
        {
            auto shm_transport = std::make_shared<SharedMemTransportDescriptor>();
            pqos.transport().user_transports.push_back(shm_transport);
        }
        else
        {
            auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
            pqos.transport().user_transports.push_back(udp_transport);
        }
    }
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    topic_ = participant_->create_topic(
        topic_name,
        "HelloWorld",
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE READER
    if (threshold > 0)
    {
        set_listener_threshold(threshold);
    }
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    if (!transport.empty())
    {
        rqos.data_sharing().off();
    }

    if (reliable)
    {
        rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        rqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    if (transient)
    {
        rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    }
    else
    {
        rqos.durability().kind = VOLATILE_DURABILITY_QOS;
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

void HelloWorldSubscriber::set_listener_threshold(
        uint32_t threshold)
{
    listener_.threshold_ = threshold;
}

void HelloWorldSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
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
    while (reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message " << hello_.message() << " " << hello_.index() << " RECEIVED" << std::endl;
            if (threshold_ > 0 && (samples_ >= threshold_))
            {
                stop = true;
                terminate_cv.notify_one();
            }
        }
    }
}

void HelloWorldSubscriber::run(
        uint32_t samples)
{
    if (samples > 0)
    {
        std::cout << "Subscriber running until " << samples << " samples have been received" << std::endl;
    }
    else
    {
        std::cout << "Subscriber running. Please press CTRL+C to stop the Subscriber" << std::endl;
    }
    stop = false;
    signal(SIGINT, [](int signum)
            {
                static_cast<void>(signum); stop = true; terminate_cv.notify_one();
            });
    std::unique_lock<std::mutex> lck(mtx);
    terminate_cv.wait(lck, []
            {
                return stop;
            });
}
