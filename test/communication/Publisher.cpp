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
 * @file Publisher.cpp
 */
#include <asio.hpp>

#include "Publisher.hpp"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

#include <fstream>
#include <string>

Publisher::~Publisher()
{
    if (participant_)
    {
        eprosima::fastrtps::Domain::removeParticipant(participant_);
    }

    if (type_)
    {
        delete type_;
    }
}

bool Publisher::init(
        uint32_t seed,
        const std::string& magic,
        bool fixed_type /* = false */)
{
    eprosima::fastrtps::ParticipantAttributes participant_attributes;
    eprosima::fastrtps::Domain::getDefaultParticipantAttributes(participant_attributes);
    participant_attributes.domainId = seed % 230;
    participant_ = eprosima::fastrtps::Domain::createParticipant(participant_attributes, this);

    if (nullptr == participant_)
    {
        return false;
    }

    // Construct a FixedSizedType if fixed type is required, defult HelloWorldType
    if (fixed_type)
    {
        type_ = new FixedSizedType();
    }
    else
    {
        type_ = new HelloWorldType();
    }

    eprosima::fastrtps::Domain::registerType(participant_, type_);

    // Generate topic name
    std::ostringstream topic;
    topic << "HelloWorldTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    // Create publisher
    eprosima::fastrtps::PublisherAttributes publisher_attributes;
    eprosima::fastrtps::Domain::getDefaultPublisherAttributes(publisher_attributes);
    publisher_attributes.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
    publisher_attributes.topic.topicDataType = type_->getName();
    publisher_attributes.topic.topicName = topic.str();
    publisher_attributes.qos.m_liveliness.lease_duration = 3;
    publisher_attributes.qos.m_liveliness.announcement_period = 1;
    publisher_attributes.qos.m_liveliness.kind = eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;
    publisher_ = eprosima::fastrtps::Domain::createPublisher(participant_, publisher_attributes, this);

    if (nullptr == publisher_)
    {
        eprosima::fastrtps::Domain::removeParticipant(participant_);
        return false;
    }

    return true;
}

void Publisher::wait_discovery(
        uint32_t how_many)
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&]
            {
                return matched_ >= how_many;
            });
}

void Publisher::run(
        uint32_t samples,
        const uint32_t loops)
{
    uint32_t current_loop = 0;
    HelloWorld data;
    data.index(1);
    data.message("HelloWorld");

    while (run_ && (loops == 0 || loops > current_loop))
    {
        publisher_->write((void*)&data);

        if (data.index() == samples)
        {
            data.index() = 1;
            ++current_loop;
        }
        else
        {
            ++data.index();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void Publisher::onParticipantDiscovery(
        eprosima::fastrtps::Participant* /*participant*/,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Publisher participant " <<     //participant->getGuid() <<
            " discovered participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
    {
        std::cout << "Publisher participant " <<     //participant->getGuid() <<
            " detected changes on participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Publisher participant " <<     //participant->getGuid() <<
            " removed participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        std::cout << "Publisher participant " <<     //participant->getGuid() <<
            " dropped participant " << info.info.m_guid << std::endl;
        if (exit_on_lost_liveliness_)
        {
            run_ = false;
        }
    }
}

#if HAVE_SECURITY
void Publisher::onParticipantAuthentication(
        eprosima::fastrtps::Participant* participant,
        eprosima::fastrtps::rtps::ParticipantAuthenticationInfo&& info)
{
    if (eprosima::fastrtps::rtps::ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
    {
        std::cout << "Publisher participant " << participant->getGuid() <<
            " authorized participant " << info.guid << std::endl;
    }
    else
    {
        std::cout << "Publisher participant " << participant->getGuid() <<
            " unauthorized participant " << info.guid << std::endl;
    }
}

#endif // if HAVE_SECURITY
