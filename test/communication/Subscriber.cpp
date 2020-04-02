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
 * @file Subscriber.cpp
 *
 */

#include <asio.hpp>

#include "Subscriber.hpp"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fstream>
#include <string>

Subscriber::~Subscriber()
{
    if (participant_)
    {
        eprosima::fastrtps::Domain::removeParticipant(participant_);
    }
}

bool Subscriber::init(
        uint32_t seed,
        const std::string& magic)
{
    eprosima::fastrtps::ParticipantAttributes participant_attributes;
    eprosima::fastrtps::Domain::getDefaultParticipantAttributes(participant_attributes);
    participant_attributes.domainId = seed % 230;
    participant_ = eprosima::fastrtps::Domain::createParticipant(participant_attributes, this);

    if (nullptr == participant_)
    {
        return false;
    }

    //Register the type
    eprosima::fastrtps::Domain::registerType(participant_, &type_);

    // Generate topic name
    std::ostringstream topic;
    topic << "HelloWorldTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    //CREATE THE SUBSCRIBER
    eprosima::fastrtps::SubscriberAttributes subscriber_attributes;
    eprosima::fastrtps::Domain::getDefaultSubscriberAttributes(subscriber_attributes);
    subscriber_attributes.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
    subscriber_attributes.topic.topicDataType = type_.getName();
    subscriber_attributes.topic.topicName = topic.str();
    subscriber_attributes.qos.m_liveliness.lease_duration = 3;
    subscriber_attributes.qos.m_liveliness.kind = eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;
    subscriber_ = eprosima::fastrtps::Domain::createSubscriber(participant_, subscriber_attributes, this);

    if (nullptr == subscriber_)
    {
        eprosima::fastrtps::Domain::removeParticipant(participant_);
        return false;
    }

    return true;
}

void Subscriber::onNewDataMessage(
        eprosima::fastrtps::Subscriber* subscriber)
{
    HelloWorld sample;
    eprosima::fastrtps::SampleInfo_t info;

    if (subscriber->takeNextData((void*)&sample, &info))
    {
        if (info.sampleKind == eprosima::fastrtps::rtps::ALIVE)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            std::cout << "Received sample (" << info.sample_identity.writer_guid() << " - " <<
                info.sample_identity.sequence_number() << "): index(" << sample.index() << "), message("
                      << sample.message() << ")" << std::endl;
            if (max_number_samples_ <= ++number_samples_[info.sample_identity.writer_guid()])
            {
                cv_.notify_all();
            }
        }
    }
}

bool Subscriber::run(
        bool notexit)
{
    return run_for(notexit, std::chrono::hours(24));
}

bool Subscriber::run_for(
        bool notexit,
        const std::chrono::milliseconds& timeout)
{
    bool returned_value = false;

    while (notexit && run_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (run_)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        returned_value = cv_.wait_for(lock, timeout, [&]
        {
            if (publishers_ < number_samples_.size())
            {
                // Will fail later.
                return true;
            }
            else if (publishers_ > number_samples_.size())
            {
                return false;
            }

            for (auto& number_samples : number_samples_)
            {
                if (max_number_samples_ > number_samples.second)
                {
                    return false;
                }
            }

            return true;
        });
    }
    else
    {
        returned_value = true;
    }


    if (publishers_ < number_samples_.size())
    {
        std::cout << "ERROR: detected more than " << publishers_ << " publishers" << std::endl;
        returned_value = false;
    }

    return returned_value;
}

void Subscriber::onParticipantDiscovery(
        eprosima::fastrtps::Participant* /*participant*/,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " discovered participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " detected changes on participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " removed participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " dropped participant " << info.info.m_guid << std::endl;
    }
}

#if HAVE_SECURITY
void Subscriber::onParticipantAuthentication(
        eprosima::fastrtps::Participant* /*participant*/,
        eprosima::fastrtps::rtps::ParticipantAuthenticationInfo&& info)
{
    if (eprosima::fastrtps::rtps::ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " authorized participant " << info.guid << std::endl;
    }
    else
    {
        std::cout << "Subscriber participant " <<         //participant->getGuid() <<
            " unauthorized participant " << info.guid << std::endl;
    }
}

#endif
