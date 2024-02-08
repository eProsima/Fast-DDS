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
 * @file TypeLookupServiceSubscriber.h
 *
 */

#ifndef _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_SUBSCRIBER_H_
#define _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_SUBSCRIBER_H_

#include <asio.hpp>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_set>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>

#include "TypeLookupServiceTestsTypes.h"

namespace eprosima {
namespace fastdds {
namespace dds {

struct SubKnownType
{
    void* type_;
    DynamicType::_ref_type dyn_type_;
    TypeSupport type_sup_;

    std::function<void(void* sample)> print_values_;
    std::function<void(DynamicData::_ref_type sample)> dyn_print_values_;
};

// Define a macro to simplify type registration
#define SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type) \
    type_creator_functions_[#Type] = std::bind(&TypeLookupServiceSubscriber::create_known_type_impl<Type, \
                    Type ## PubSubType>, \
                    this, \
                    std::placeholders::_1)

class TypeLookupServiceSubscriber
    : public DomainParticipantListener
{
public:

    TypeLookupServiceSubscriber()
    {
    }

    ~TypeLookupServiceSubscriber();

    void create_type_creator_functions();

    bool init(
            std::vector<std::string> known_types);

    bool setup_subscriber(
            SubKnownType& new_type);

    bool create_known_type(
            const std::string& type);

    template <typename Type, typename TypePubSubType>
    bool create_known_type_impl(
            const std::string& type);

    bool create_discovered_type(
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& info);

    bool check_registered_type(
            const xtypes::TypeInformationParameter& type_info);

    bool wait_discovery(
            uint32_t expected_matches,
            uint32_t timeout);

    bool run(
            uint32_t samples,
            uint32_t timeout);

    void on_subscription_matched(
            DataReader* /*reader*/,
            const SubscriptionMatchedStatus& info) override;

    void on_data_available(
            DataReader* reader) override;

    void on_data_writer_discovery(
            eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            eprosima::fastdds::rtps::WriterDiscoveryStatus reason,
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& info,
            bool& should_be_ignored) override;

private:

    DomainParticipant* participant_ = nullptr;

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_ = 0;
    unsigned int expected_matches_ = 0;
    std::map<eprosima::fastdds::rtps::GUID_t, uint32_t> received_samples_;

    std::mutex known_types_mutex_;
    std::map<std::string, SubKnownType> known_types_;
    std::map<std::string, std::function<bool(const std::string&)>> type_creator_functions_;
    std::vector<std::thread> create_types_threads;
};

} // dds
} // fastdds
} // eprosima


#endif /* _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_SUBSCRIBER_H_ */
