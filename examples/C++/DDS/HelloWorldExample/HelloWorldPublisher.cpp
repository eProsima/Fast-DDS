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
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/utils/eClock.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/DataWriter.hpp>

#include <thread>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldPublisher::HelloWorldPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldPublisher::init()
{
    hello_.index(0);
    hello_.message("HelloWorld");
    ParticipantAttributes participant_att;
    participant_att.rtps.builtin.domainId = 0;
    participant_att.rtps.setName("Participant_pub");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(participant_att);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_, type_->getName());

    //CREATE THE PUBLISHER
    PublisherAttributes pub_att;
    pub_att.topic.topicDataType = "HelloWorld";
    pub_att.topic.topicName = "HelloWorldTopic";
    pub_att.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    pub_att.topic.historyQos.depth = 30;
    pub_att.topic.resourceLimitsQos.max_samples = 50;
    pub_att.topic.resourceLimitsQos.allocated_samples = 20;
    pub_att.times.heartbeatPeriod.seconds = 2;
    pub_att.times.heartbeatPeriod.nanosec = 200*1000*1000;
    pub_att.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, pub_att, nullptr);

    if (publisher_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    writer_ = publisher_->create_datawriter(pub_att.topic, pub_att.qos, &listener_);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        eprosima::fastdds::dds::PublicationMatchedStatus &info)
{
    if(info.status == MATCHED_MATCHING)
    {
        matched_++;
        firstConnected_ = true;
        std::cout << "Publisher matched." << std::endl;
    }
    else
    {
        matched_--;
        std::cout << "Publisher unmatched." << std::endl;
    }
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
            eClock::my_sleep(sleep);
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
            eClock::my_sleep(sleep);
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

bool HelloWorldPublisher::publish(bool waitForListener)
{
    if(listener_.firstConnected_ || !waitForListener || listener_.matched_>0)
    {
        hello_.index(hello_.index()+1);
        writer_->write(&hello_);
        return true;
    }
    return false;
}
