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
 * @file TypeLookupPublisher.h
 *
 */

#ifndef _TYPELOOKUPTEST_PUBLISHER_H_
#define _TYPELOOKUPTEST_PUBLISHER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

#include "idl/XtypesTestsType1PubSubTypes.h"
#include "idl/XtypesTestsType2PubSubTypes.h"
#include "idl/XtypesTestsType3PubSubTypes.h"

#include <mutex>
#include <condition_variable>
#include <stdexcept>

namespace eprosima {
namespace fastdds {
namespace dds {

class TypeLookupPublisherTypeNotRegisteredException : public std::runtime_error
{
public:

    TypeLookupPublisherTypeNotRegisteredException(
            std::string type_name)
        : std::runtime_error("Type: " + type_name + " not registered in TypeLookupPublisher")
    {
    }

};

struct PubKnownType
{
    TypeSupport type_;
    void* obj_;
    void* type_sup_;
    Publisher* publisher_ = nullptr;
    DataWriter* writer_ = nullptr;
    Topic* topic_ = nullptr;

    std::function<void(void* data, int current_sample)> callback_;
};

class TypeLookupPublisher
    : public DomainParticipantListener
{
public:

    TypeLookupPublisher()
    {
    }

    ~TypeLookupPublisher();

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

    void on_publication_matched(
            DataWriter* /*writer*/,
            const PublicationMatchedStatus& info) override;

    void on_data_reader_discovery(
            eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info) override;

private:

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_ = 0;

    std::map<eprosima::fastrtps::rtps::GUID_t, uint32_t> sent_samples_;

    DomainParticipant* participant_ = nullptr;

    std::map<std::string, PubKnownType> known_types_;
};

} // dds
} // fastdds
} // eprosima

#endif /* _TYPELOOKUPTEST_PUBLISHER_H_ */