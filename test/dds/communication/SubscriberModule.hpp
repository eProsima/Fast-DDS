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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

#include "types/FixedSizedType.h"
#include "types/HelloWorldType.h"

#include <mutex>
#include <condition_variable>
#include <map>
#include <chrono>

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
            bool fixed_type = false,
            bool zero_copy = false)
        : publishers_(publishers)
        , max_number_samples_(max_number_samples)
        , fixed_type_(fixed_type)
        , zero_copy_(zero_copy)
    {
    }

    ~SubscriberModule();

    void on_participant_discovery(
            DomainParticipant* /*participant*/,
            fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

#if HAVE_SECURITY
    void onParticipantAuthentication(
            DomainParticipant* /*participant*/,
            fastrtps::rtps::ParticipantAuthenticationInfo&& info) override;
#endif

    void on_subscription_matched(
            DataReader* /*reader*/,
            const SubscriptionMatchedStatus& info) override;

    void on_data_available(
            DataReader* reader) override;

    void on_liveliness_changed(
            DataReader* /*reader*/,
            const eprosima::fastdds::dds::LivelinessChangedStatus& status) override;

    void on_sample_rejected(
            DataReader* /*reader*/,
            const fastrtps::SampleRejectedStatus& /*status*/) override;

    void on_sample_lost(
            DataReader* /*reader*/,
            const SampleLostStatus& /*status*/) override;

    bool init(
            uint32_t seed,
            const std::string& magic);

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
    bool fixed_type_ = false;
    bool zero_copy_ = false;
    bool run_ = true;
    DomainParticipant* participant_ = nullptr;
    TypeSupport* type_;
    Subscriber* subscriber_ = nullptr;
    DataReader* reader_ = nullptr;
    Topic* topic_ = nullptr;
};

} // dds
} // fastdds
} // eprosima

#endif // TEST_COMMUNICATION_SUBSCRIBER_HPP
