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

#include "types/FixedSizedType.h"
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
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

#if HAVE_SECURITY
    void onParticipantAuthentication(
            eprosima::fastrtps::Participant* participant,
            eprosima::fastrtps::rtps::ParticipantAuthenticationInfo&& info) override;
#endif

    bool init(
            uint32_t seed,
            const std::string& magic,
            bool fixed_type = false);

    void wait_discovery(
            uint32_t how_many);

    void run(
            uint32_t samples,
            uint32_t loops = 0);

private:

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_ = 0;
    bool exit_on_lost_liveliness_ = false;
    bool run_ = true;
    eprosima::fastrtps::Participant* participant_ = nullptr;
    eprosima::fastrtps::TopicDataType* type_;
    eprosima::fastrtps::Publisher* publisher_ = nullptr;
};

#endif // TEST_COMMUNICATION_PUBLISHER_HPP
