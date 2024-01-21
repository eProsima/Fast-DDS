// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _TYPELOOKUPTEST_SUBSCRIBER_H_
#define _TYPELOOKUPTEST_SUBSCRIBER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

#include "TypeLookupTestsTypes.h"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <map>
#include <mutex>

namespace eprosima {
namespace fastdds {
namespace dds {

class TypeLookupSubscriberTypeNotRegisteredException : public std::runtime_error
{
public:

    TypeLookupSubscriberTypeNotRegisteredException(
            std::string type_name)
        : std::runtime_error("Type: " + type_name + " not registered in TypeLookupSubscriber")
    {
    }

};

struct SubKnownType
{
    TypeSupport type_;
    void* obj_;
    void* type_sup_;
    Subscriber* subscriber_ = nullptr;
    DataReader* reader_ = nullptr;
    Topic* topic_ = nullptr;

    std::function<void(void* data)> callback_;
};

// Define a macro to simplify type registration
#define REGISTER_TYPE(Type) \
    type_creator_functions_[#Type] = std::bind(&TypeLookupSubscriber::create_known_type_impl<Type, Type ## PubSubType>, \
                    this, \
                    std::placeholders::_1)

class TypeLookupSubscriber
    : public DomainParticipantListener
{
public:

    TypeLookupSubscriber()
    {
    }

    ~TypeLookupSubscriber();

    bool init(
            std::vector<std::string> known_types);

    bool create_known_type(
            const std::string& type);

    template <typename Type, typename TypePubSubType>
    bool create_known_type_impl(
            const std::string& type);

    bool check_registered_type(
            const xtypes::TypeInformationParameter& type_info);

    bool wait_discovery(
            uint32_t expected_match,
            uint32_t timeout);

    bool run(
            uint32_t samples);

    void on_subscription_matched(
            DataReader* /*reader*/,
            const SubscriptionMatchedStatus& info) override;

    void on_data_available(
            DataReader* reader) override;

    void on_data_writer_discovery(
            eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info) override;

private:

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_ = 0;

    std::mutex received_samples_mutex_;
    std::map<eprosima::fastrtps::rtps::GUID_t, uint32_t> received_samples_;

    DomainParticipant* participant_ = nullptr;

    std::map<std::string, std::function<bool(const std::string&)>> type_creator_functions_;
    std::map<std::string, SubKnownType> known_types_;
};

} // dds
} // fastdds
} // eprosima


#endif /* _TYPELOOKUPTEST_SUBSCRIBER_H_ */