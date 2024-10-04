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
 * @file SubscriberModule.hpp
 *
 */
#ifndef TEST_COMMUNICATION_SUBSCRIBER_HPP
#define TEST_COMMUNICATION_SUBSCRIBER_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>

#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>

#include "types/FixedSizedPubSubTypes.hpp"
#include "types/HelloWorldPubSubTypes.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class SubscriberModule
    : public DomainParticipantListener
{
public:

    SubscriberModule(
            const uint32_t publishers,
            const uint32_t max_number_samples,
            bool fixed_type,
            bool zero_copy,
            bool die_on_data_received)
        : publishers_(publishers)
        , max_number_samples_(max_number_samples)
        , fixed_type_(zero_copy || fixed_type) // If zero copy active, fixed type is required
        , zero_copy_(zero_copy)
        , die_on_data_received_(die_on_data_received)
    {
    }

    ~SubscriberModule();

    void on_participant_discovery(
            DomainParticipant* /*participant*/,
            fastdds::rtps::ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData& info,
            bool& should_be_ignored) override;

#if HAVE_SECURITY
    void onParticipantAuthentication(
            DomainParticipant* /*participant*/,
            fastdds::rtps::ParticipantAuthenticationInfo&& info) override;
#endif // if HAVE_SECURITY

    void on_subscription_matched(
            DataReader* /*reader*/,
            const SubscriptionMatchedStatus& info) override;

    void on_data_available(
            DataReader* reader) override;

    void on_liveliness_changed(
            DataReader* /*reader*/,
            const eprosima::fastdds::dds::LivelinessChangedStatus& status) override;

    bool init(
            uint32_t seed,
            const std::string& magic);

    bool run(
            bool notexit,
            const uint32_t rescan_interval);

    bool run_for(
            bool notexit,
            const uint32_t rescan_interval,
            const std::chrono::milliseconds& timeout);

private:

    using DomainParticipantListener::on_participant_discovery;

    std::mutex mutex_;
    std::condition_variable cv_;
    const uint32_t publishers_ = 0;
    const uint32_t max_number_samples_ = 0;
    std::map<eprosima::fastdds::rtps::GUID_t, uint32_t> number_samples_;
    bool fixed_type_ = false;
    bool zero_copy_ = false;
    std::atomic_bool run_{true};
    DomainParticipant* participant_ = nullptr;
    TypeSupport type_;
    Subscriber* subscriber_ = nullptr;
    DataReader* reader_ = nullptr;
    Topic* topic_ = nullptr;
    bool die_on_data_received_ = false;
};

} // dds
} // fastdds
} // eprosima

#endif // TEST_COMMUNICATION_SUBSCRIBER_HPP
