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
 * @file Publisher.hpp
 */

#ifndef TEST_DDS_COMMUNICATION_PUBLISHERMODULE_HPP
#define TEST_DDS_COMMUNICATION_PUBLISHERMODULE_HPP

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

#include "types/FixedSizedType.h"
#include "types/HelloWorldType.h"

#include <mutex>
#include <condition_variable>

namespace eprosima {
namespace fastdds {
namespace dds {

class PublisherModule
    : public DomainParticipantListener
{
public:

    PublisherModule(
            bool exit_on_lost_liveliness,
            bool fixed_type = false,
            bool zero_copy = false)
        : exit_on_lost_liveliness_(exit_on_lost_liveliness)
        , fixed_type_(zero_copy || fixed_type) // If zero copy active, fixed type is required
        , zero_copy_(zero_copy)
    {
    }

    ~PublisherModule();

    void on_publication_matched(
            DataWriter* /*publisher*/,
            const PublicationMatchedStatus& info) override;

    /**
     * This method is called when a new Participant is discovered, or a previously discovered participant
     * changes its QOS or is removed.
     * @param p Pointer to the Participant
     * @param info DiscoveryInfo.
     */
    void on_participant_discovery(
            DomainParticipant* /*participant*/,
            fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

#if HAVE_SECURITY
    void onParticipantAuthentication(
            DomainParticipant* participant,
            fastrtps::rtps::ParticipantAuthenticationInfo&& info) override;
#endif // if HAVE_SECURITY

    bool init(
            uint32_t seed,
            const std::string& magic);

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
    bool fixed_type_ = false;
    bool zero_copy_ = false;
    bool run_ = true;
    DomainParticipant* participant_ = nullptr;
    TypeSupport type_;
    Publisher* publisher_ = nullptr;
    DataWriter* writer_ = nullptr;
    Topic* topic_ = nullptr;
};

} // dds
} // fastdds
} // eprosima

#endif // TEST_DDS_COMMUNICATION_PUBLISHERMODULE_HPP
