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
 * @file TypeLookupServicePublisher.h
 *
 */

#ifndef _TYPELOOKUPSERVICETEST_PUBLISHER_H_
#define _TYPELOOKUPSERVICETEST_PUBLISHER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

#include "TypeLookupServiceTestsTypes.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <map>
#include <future>

namespace eprosima {
namespace fastdds {
namespace dds {

class TypeLookupServicePublisherTypeRegistryException : public std::runtime_error
{
public:

    TypeLookupServicePublisherTypeRegistryException(
            std::string msg)
        : std::runtime_error("Type: " + msg + " in TypeLookupServicePublisher")
    {
    }

};

struct PubKnownType
{
    TypeSupport type_;
    void* type_sup_;
    Publisher* publisher_ = nullptr;
    DataWriter* writer_ = nullptr;
    Topic* topic_ = nullptr;

    std::function<void(void* data, int current_sample)> callback_;
};

// Define a macro to simplify type registration
#define PUBLISHER_TYPE_CREATOR_FUNCTION(Type) \
    type_creator_functions_[#Type] = std::bind(&TypeLookupServicePublisher::create_known_type_impl<Type, \
                    Type ## PubSubType>, \
                    this, \
                    std::placeholders::_1)


class TypeLookupServicePublisher
    : public DomainParticipantListener
{
public:

    TypeLookupServicePublisher()
    {
    }

    ~TypeLookupServicePublisher();

    void create_type_creator_functions();

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
            uint32_t expected_matches,
            uint32_t timeout);

    bool run(
            uint32_t samples,
            uint32_t timeout);

    void on_publication_matched(
            DataWriter* /*writer*/,
            const PublicationMatchedStatus& info) override;

    void on_data_reader_discovery(
            eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info) override;

private:

    DomainParticipant* participant_ = nullptr;

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_ = 0;
    unsigned int expected_matches_ = 0;

    std::map<eprosima::fastrtps::rtps::GUID_t, uint32_t> sent_samples_;

    std::mutex known_types_mutex_;
    std::map<std::string, PubKnownType> known_types_;
    std::map<std::string, std::function<bool(const std::string&)>> type_creator_functions_;
    std::vector<std::thread> create_known_types_threads;
};

} // dds
} // fastdds
} // eprosima

#endif /* _TYPELOOKUPSERVICETEST_PUBLISHER_H_ */