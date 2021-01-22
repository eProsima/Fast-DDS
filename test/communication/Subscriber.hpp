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
 * @file Subscriber.hpp
 *
 */
#ifndef TEST_COMMUNICATION_SUBSCRIBER_HPP
#define TEST_COMMUNICATION_SUBSCRIBER_HPP

#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include "types/FixedSizedType.h"
#include "types/HelloWorldType.h"

#include <mutex>
#include <condition_variable>
#include <map>
#include <chrono>

namespace eprosima {
namespace fastrtps {
class Participant;
class Subscriber;
}
}

class Subscriber
    : public eprosima::fastrtps::SubscriberListener
    , public eprosima::fastrtps::ParticipantListener
{
public:

    Subscriber(
            const uint32_t publishers,
            const uint32_t max_number_samples,
            bool zero_copy = false)
        : publishers_(publishers)
        , max_number_samples_(max_number_samples)
        , zero_copy_(zero_copy)
    {
    }

    ~Subscriber();

    void onParticipantDiscovery(
            eprosima::fastrtps::Participant* /*participant*/,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

#if HAVE_SECURITY
    void onParticipantAuthentication(
            eprosima::fastrtps::Participant* /*participant*/,
            eprosima::fastrtps::rtps::ParticipantAuthenticationInfo&& info) override;
#endif

    void onSubscriptionMatched(
            eprosima::fastrtps::Subscriber* /*subscriber*/,
            eprosima::fastrtps::rtps::MatchingInfo& info) override
    {
        if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
        {
            std::cout << "Subscriber matched with publisher " << info.remoteEndpointGuid << std::endl;
        }
        else
        {
            std::cout << "Subscriber unmatched with publisher " << info.remoteEndpointGuid << std::endl;
        }
    }

    void onNewDataMessage(
            eprosima::fastrtps::Subscriber* subscriber) override;

    void on_liveliness_changed(
            eprosima::fastrtps::Subscriber* sub,
            const eprosima::fastrtps::LivelinessChangedStatus& status) override
    {
        (void)sub;
        if (status.alive_count_change == 1)
        {
            std::cout << "Publisher recovered liveliness" << std::endl;
        }
        else if (status.not_alive_count_change == 1)
        {
            std::cout << "Publisher lost liveliness" << std::endl;
            run_ = false;
        }
    }

    bool init(
            uint32_t seed,
            const std::string& magic,
            bool fixed_type = false);

    bool run(
            bool notexit);

    bool run_for(
            bool notexit,
            const std::chrono::milliseconds& timeout);

private:

    std::mutex mutex_;
    std::condition_variable cv_;
    const uint32_t publishers_ = 0;
    const uint32_t max_number_samples_ = 0;
    std::map<eprosima::fastrtps::rtps::GUID_t, uint32_t> number_samples_;
    bool run_ = true;
    eprosima::fastrtps::Participant* participant_ = nullptr;
    eprosima::fastrtps::TopicDataType* type_;
    eprosima::fastrtps::Subscriber* subscriber_ = nullptr;
    bool zero_copy_;
};
#endif // TEST_COMMUNICATION_SUBSCRIBER_HPP
