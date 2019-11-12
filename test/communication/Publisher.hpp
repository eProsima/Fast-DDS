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
 * @file Publisher.hpp
 */

#ifndef TEST_COMMUNICATION_PUBLISHER_HPP
#define TEST_COMMUNICATION_PUBLISHER_HPP

#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/participant/ParticipantListener.h>

#include "types/HelloWorldType.h"

#include <mutex>
#include <condition_variable>

namespace eprosima {
namespace fastrtps {
class Participant;
class Publisher;
}
}

class Publisher
    : public eprosima::fastrtps::PublisherListener
    , public eprosima::fastrtps::ParticipantListener
{
public:

    Publisher(
            bool exit_on_lost_liveliness)
        : exit_on_lost_liveliness_(exit_on_lost_liveliness)
    {
    }

    ~Publisher();

    void onPublicationMatched(
            eprosima::fastrtps::Publisher* /*publisher*/,
            eprosima::fastrtps::rtps::MatchingInfo& info) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
        {
            std::cout << "Publisher matched with subscriber " << info.remoteEndpointGuid << std::endl;
            ++matched_;
        }
        else
        {
            std::cout << "Publisher unmatched with subscriber " << info.remoteEndpointGuid << std::endl;
            --matched_;
        }
        cv_.notify_all();
    }

    /**
     * This method is called when a new Participant is discovered, or a previously discovered participant
     * changes its QOS or is removed.
     * @param p Pointer to the Participant
     * @param info DiscoveryInfo.
     */
    void onParticipantDiscovery(
            eprosima::fastrtps::Participant* /*participant*/,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override
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
    void onParticipantAuthentication(
            eprosima::fastrtps::Participant* participant,
            eprosima::fastrtps::rtps::ParticipantAuthenticationInfo&& info) override
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

#endif

    bool init(
            uint32_t seed,
            const std::string& magic);

    void wait_discovery(
            uint32_t how_many);

    void run(
            uint32_t samples);

private:

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_ = 0;
    bool exit_on_lost_liveliness_ = false;
    bool run_ = true;
    eprosima::fastrtps::Participant* participant_ = nullptr;
    HelloWorldType type_;
    eprosima::fastrtps::Publisher* publisher_ = nullptr;
};

#endif // TEST_COMMUNICATION_PUBLISHER_HPP
