// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "HelloWorldPublisher.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/topic/qos/TopicQos.hpp>
#include <dds/core/policy/CorePolicy.hpp>

#include <thread>

using namespace eprosima::fastdds::dds;

HelloWorldPublisher::HelloWorldPublisher()
    : participant_(nullptr)
    , publisher_(dds::core::null)
    , writer_(dds::core::null)
    , topic_(dds::core::null)
{
}

bool HelloWorldPublisher::init()
{
    hello_.index(0);
    hello_.message("HelloWorld");

    participant_ = dds::domain::DomainParticipant(0);

    if (participant_ == dds::core::null)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_.delegate().get(), type_.get_type_name());

    // TopicQos
    dds::topic::qos::TopicQos topicQos
        = participant_.default_topic_qos()
            << dds::core::policy::Reliability::Reliable();

    topic_ = dds::topic::Topic<HelloWorld>(participant_, "HelloWorldTopic", "HelloWorld", topicQos);

    //CREATE THE PUBLISHER
    publisher_ = ::dds::pub::Publisher(participant_);

    if (publisher_ == dds::core::null)
    {
        return false;
    }

    // DataWriterQos
    dds::pub::qos::DataWriterQos dwqos = topicQos;

    // CREATE THE WRITER
    writer_ = dds::pub::DataWriter<HelloWorld>(publisher_, topic_, dwqos, &listener_);

    if (writer_ == dds::core::null)
    {
        return false;
    }

    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    participant_.delete_participant();
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        dds::pub::DataWriter<HelloWorld>& writer,
        const dds::core::status::PublicationMatchedStatus& status)
{
    on_publication_matched(writer.delegate().get(), status);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        firstConnected_ = true;
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
}

void HelloWorldPublisher::PubListener::on_offered_incompatible_qos(
        DataWriter*,
        const OfferedIncompatibleQosStatus& status)
{
    QosPolicy qos;
    std::cout << "The Offered Qos is incompatible with the Requested one." << std::endl;
    std::cout << "The Qos causing this incompatibility is " << qos.search_qos_by_id(
        status.last_policy_id) << "." << std::endl;
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    if (samples == 0)
    {
        while (!stop_)
        {
            if (publish(false))
            {
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
    else
    {
        for (uint32_t i = 0; i < samples; ++i)
        {
            if (!publish())
            {
                --i;
            }
            else
            {
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
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
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop_ = true;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

bool HelloWorldPublisher::publish(
        bool waitForListener)
{
    if (listener_.firstConnected_ || !waitForListener || listener_.matched_>0)
    {
        hello_.index(hello_.index()+1);
        writer_.write(hello_);
        return true;
    }
    return false;
}
