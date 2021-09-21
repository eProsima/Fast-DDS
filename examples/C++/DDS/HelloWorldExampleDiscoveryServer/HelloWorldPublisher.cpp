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
 * @file HelloWorldPublisher.cpp
 *
 */

#include <csignal>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include "HelloWorldPublisher.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> HelloWorldPublisher::stop_(false);
std::mutex HelloWorldPublisher::PubListener::wait_matched_cv_mtx_;
std::condition_variable HelloWorldPublisher::PubListener::wait_matched_cv_;

HelloWorldPublisher::HelloWorldPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
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
}

bool HelloWorldPublisher::init(
        const std::string& topic_name,
        uint32_t num_wait_matched,
        eprosima::fastdds::rtps::Locator server_address)
{
    hello_.index(0);
    hello_.message("HelloWorld");
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    listener_.set_num_wait_matched(num_wait_matched);

    // Set participant as DS CLIENT
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol_t::CLIENT;

    // Set SERVER's GUID prefix
    RemoteServerAttributes remote_server_att;
    remote_server_att.ReadguidPrefix("44.53.00.5f.45.50.52.4f.53.49.4d.41");

    // Set SERVER's listening locator for PDP
    remote_server_att.metatrafficUnicastLocatorList.push_back(server_address);

    // Add remote SERVER to CLIENT's list of SERVERs
    pqos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(remote_server_att);

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

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

    // CREATE THE TOPIC
    topic_ = participant_->create_topic(topic_name, "HelloWorld", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    writer_ = publisher_->create_datawriter(topic_, wqos, &listener_);

    if (writer_ == nullptr)
    {
        return false;
    }
    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    if (participant_ != nullptr)
    {
        if (publisher_ != nullptr)
        {
            if (writer_ != nullptr)
            {
                publisher_->delete_datawriter(writer_);
            }
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher matched." << std::endl;
        if (enough_matched())
        {
            awake();
        }
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldPublisher::PubListener::set_num_wait_matched(
        uint32_t num_wait_matched)
{
    num_wait_matched_ = num_wait_matched;
}

bool HelloWorldPublisher::PubListener::enough_matched()
{
    return matched_ >= num_wait_matched_;
}

void HelloWorldPublisher::PubListener::wait()
{
    std::unique_lock<std::mutex> lck(wait_matched_cv_mtx_);
    wait_matched_cv_.wait(lck, [this]
            {
                return enough_matched() || is_stopped();
            });
}

void HelloWorldPublisher::PubListener::awake()
{
    wait_matched_cv_.notify_all();
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    while (!is_stopped() && (samples == 0 || hello_.index() < samples))
    {
        if (listener_.enough_matched())
        {
            publish();
            std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                      << " SENT" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
        else
        {
            listener_.wait();
        }
    }
}

void HelloWorldPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop_ = false;
    std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    signal(SIGINT, [](int signum)
            {
                static_cast<void>(signum); HelloWorldPublisher::stop();
            });
    thread.join();
}

void HelloWorldPublisher::publish()
{
    hello_.index(hello_.index() + 1);
    writer_->write(&hello_);
}
