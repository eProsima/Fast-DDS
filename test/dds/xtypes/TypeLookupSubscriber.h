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
 * @file TypeLookupSubscriber.h
 *
 */

#ifndef _TEST_SUBSCRIBER_H_
#define _TEST_SUBSCRIBER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

#include "idl/XtypesTestsTypesPubSubTypes.h"

#include <mutex>
#include <condition_variable>
#include <map>
#include <chrono>

namespace eprosima {
namespace fastdds {
namespace dds {

class TypeLookupSubscriber
    : public DomainParticipantListener
{
public:

    TypeLookupSubscriber()
    {
    }

    ~TypeLookupSubscriber();

    bool init();

    bool create_type();

    bool run();

    bool run_for(
            const std::chrono::milliseconds& timeout);

    void on_subscription_matched(
            DataReader* /*reader*/,
            const SubscriptionMatchedStatus& info) override;

    void on_data_available(
            DataReader* reader) override;

private:

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_ = 0;
    const uint32_t publishers_ = 1;
    const uint32_t max_number_samples_ = 10;
    std::map<eprosima::fastrtps::rtps::GUID_t, uint32_t> number_samples_;
    bool run_ = true;
    DomainParticipant* participant_ = nullptr;
    TypeSupport type_;
    Subscriber* subscriber_ = nullptr;
    DataReader* reader_ = nullptr;
    Topic* topic_ = nullptr;
};

} // dds
} // fastdds
} // eprosima


#endif /* _TEST_SUBSCRIBER_H_ */
