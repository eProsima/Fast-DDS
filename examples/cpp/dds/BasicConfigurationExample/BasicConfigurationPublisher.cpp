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
 * @file BasicConfigurationPublisher.cpp
 *
 */

#include <csignal>
#include <thread>
#include <algorithm>
#include <array>
#include <random>
#include <chrono>
#include <numeric>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include "BasicConfigurationPublisher.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<uint32_t> HelloWorldPublisher::n_topics_(0);
std::atomic<bool> HelloWorldPublisher::stop_(false);
std::mutex PubListener::wait_matched_cv_mtx_;
std::condition_variable PubListener::wait_matched_cv_;
std::mutex HelloWorldPublisher::terminate_cv_mtx_;
std::condition_variable HelloWorldPublisher::terminate_cv_;

HelloWorldPublisher::HelloWorldPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldPublisher::is_stopped()
{
    return stop_;
}

void HelloWorldPublisher::stop()
{
    stop_ = true;
    PubListener::awake();
    terminate_cv_.notify_all();
}

bool HelloWorldPublisher::init(
        std::vector<std::string> topic_names,
        uint32_t domain,
        uint32_t num_wait_matched,
        bool async,
        TransportType transport,
        bool reliable,
        bool transient,
        bool realloc)
{
    n_topics_.store(topic_names.size());

    for (uint32_t i = 0; i < n_topics_.load(); i++)
    {
        HelloWorld hello;
        hello.index(0);
        memcpy(hello.message().data(), "HelloWorld ", strlen("HelloWorld") + 1);
        hellos_.push_back(hello);
    }

    DomainParticipantQos pqos;
    pqos.name("Participant_pub");

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

    // CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (publisher_ == nullptr)
    {
        return false;
    }

    // CONFIGURE WRITER QOS
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;

    // Data sharing set in endpoint. If it is not default, set it to off
    if (transport != DEFAULT)
    {
        wqos.data_sharing().off();
    }
    else
    {
        wqos.data_sharing().automatic();  // default
    }

    if (async)
    {
        wqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    }
    else
    {
        wqos.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;    // default
    }

    if (reliable)
    {
        wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        wqos.history().kind = KEEP_ALL_HISTORY_QOS;
    }
    else
    {
        wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;  // default in this example (although default value for
                                                                // writters' qos actually is RELIABLE)
    }

    if (transient)
    {
        wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        wqos.history().kind = KEEP_ALL_HISTORY_QOS;     // store previously sent samples so they can be resent to newly
                                                        // matched DataReaders
    }
    else
    {
        wqos.durability().kind = VOLATILE_DURABILITY_QOS;   // default in this example (although default value for
                                                            // writters' qos actually is TRANSIENT_LOCAL)
    }

    if (realloc)
    {
        wqos.endpoint().history_memory_policy =
                eprosima::fastrtps::rtps::MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }

    for (uint32_t i = 0; i < n_topics_.load(); i++)
    {
        std::string topic_name = topic_names[i];
        std::shared_ptr<PubListener> listener = std::make_shared<PubListener>();
        listener->topic_name_ = topic_name;

        // CREATE THE TOPIC
        eprosima::fastdds::dds::Topic* topic = participant_->create_topic(topic_name, "HelloWorld", TOPIC_QOS_DEFAULT);

        if (topic == nullptr)
        {
            return false;
        }

        topics_.push_back(topic);

        // CREATE THE WRITER
        listener->set_num_wait_matched(num_wait_matched);
        listeners_.push_back(listener);

        eprosima::fastdds::dds::DataWriter* writer = publisher_->create_datawriter(topic, wqos, listener.get());

        if (writer == nullptr)
        {
            return false;
        }

        writers_.push_back(writer);
    }

    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
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
        if (publisher_ != nullptr)
        {
            for (uint32_t i = 0; i < n_topics_.load(); i++)
            {
                if (writers_[i] != nullptr)
                {
                    publisher_->delete_datawriter(writers_[i]);
                }
            }
            participant_->delete_publisher(publisher_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        // std::cout << "Publisher matched in " << topic_name_ << std::endl;
        logWarning(BASIC_CONFIGURATION_PUBLISHER,
                "Thread: " << std::this_thread::get_id() << " | " << "Publisher matched in " << topic_name_);
        if (enough_matched())
        {
            awake();
        }
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        // std::cout << "Publisher unmatched in " << topic_name_  << std::endl;
        logWarning(BASIC_CONFIGURATION_PUBLISHER,
                "Thread: " << std::this_thread::get_id() << " | " << "Publisher unmatched in " << topic_name_);
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
        logWarning(BASIC_CONFIGURATION_PUBLISHER,
                info.current_count_change <<
                " is not a valid value for PublicationMatchedStatus current count change");
    }
}

void PubListener::set_num_wait_matched(
        uint32_t num_wait_matched)
{
    num_wait_matched_ = num_wait_matched;
}

bool PubListener::enough_matched()
{
    return matched_ >= num_wait_matched_;
}

void PubListener::wait()
{
    std::unique_lock<std::mutex> lck(wait_matched_cv_mtx_);
    wait_matched_cv_.wait(lck, [this]
            {
                return enough_matched() || HelloWorldPublisher::is_stopped();
            });
}

void PubListener::awake()
{
    wait_matched_cv_.notify_all();
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep,
        uint32_t idx)
{
    while (!is_stopped() && (samples == 0 || hellos_[idx].index() < samples))
    {
        if (listeners_[idx]->enough_matched())
        {
            publish(idx);
            // std::cout << "Message: " << hellos_[idx].message().data() << " with index: " << hellos_[idx].index()
            //             << " SENT in " << listeners_[idx]->topic_name_ << std::endl;
            logWarning(BASIC_CONFIGURATION_PUBLISHER, "Thread: " << std::this_thread::get_id() << " | " <<
                    "Message: " << hellos_[idx].message().data() << " with index: " << hellos_[idx].index() << " SENT in " <<
                    listeners_[idx]->topic_name_);
            if (samples == 0 || hellos_[idx].index() < samples)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
            }
        }
        else
        {
            listeners_[idx]->wait();
        }
    }
}

void HelloWorldPublisher::runSingleThread(
        uint32_t samples,
        uint32_t sleep,
        uint32_t init_sleep,
        bool random)
{
    uint32_t n_topics = n_topics_.load();
    uint32_t n_finished = 0;
    std::vector<uint32_t> send_order(n_topics);
    std::iota(send_order.begin(), send_order.end(), 0);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    if (init_sleep > 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(init_sleep));
    }
    while (!is_stopped() && n_finished < n_topics)
    {
        if (random)
        {
            shuffle(send_order.begin(), send_order.end(), rng);
        }
        for (uint32_t i = 0; i < n_topics; i++)
        {
            uint32_t idx = send_order[i];
            if (listeners_[idx]->enough_matched() && (samples == 0 || hellos_[idx].index() < samples))
            {
                publish(idx);
                // std::cout << "Message: " << hellos_[idx].message().data() << " with index: " << hellos_[idx].index()
                //         << " SENT in " << listeners_[idx]->topic_name_ << std::endl;
                logWarning(BASIC_CONFIGURATION_PUBLISHER, "Thread: " << std::this_thread::get_id() << " | " <<
                        "Message: " << hellos_[idx].message().data() << " with index: " << hellos_[idx].index() << " SENT in " <<
                        listeners_[idx]->topic_name_);
                if (hellos_[idx].index() == samples)
                {
                    n_finished++;
                }
            }
        }
        if (n_finished < n_topics)
        {
            std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
            terminate_cv_.wait_for(lck, std::chrono::milliseconds(sleep), []
                    {
                        return is_stopped();
                    });
        }
    }
}

void HelloWorldPublisher::run(
        uint32_t samples,
        uint32_t sleep,
        uint32_t init_sleep,
        bool single_thread,
        bool random)
{
    stop_ = false;

    std::vector<std::thread> threads;
    if (single_thread)
    {
        threads.push_back(std::thread(&HelloWorldPublisher::runSingleThread, this, samples, sleep, init_sleep, random));
    }
    else
    {
        for (uint32_t i = 0; i < n_topics_.load(); i++)
        {
            threads.push_back(std::thread(&HelloWorldPublisher::runThread, this, samples, sleep, i));
        }
    }

    if (samples == 0)
    {
        std::cout << "Publisher running. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }
    else
    {
        std::cout << "Publisher running " << samples <<
            " samples. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }

    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Publisher execution." << std::endl;
                static_cast<void>(signum); HelloWorldPublisher::stop();
            });

    if (single_thread)
    {
        threads[0].join();
    }
    else
    {
        for (uint32_t i = 0; i < n_topics_.load(); i++)
        {
            threads[i].join();
        }
    }
}

void HelloWorldPublisher::publish(
        uint32_t writer_idx)
{
    hellos_[writer_idx].index(hellos_[writer_idx].index() + 1);
    writers_[writer_idx]->write(&hellos_[writer_idx]);
}
