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

#ifndef _TEST_PUBLISHER_H_
#define _TEST_PUBLISHER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

#include "idl/XtypesTestsTypesPubSubTypes.h"

// #include <rtps/RTPSDomainImpl.hpp>
// #include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

#include <mutex>
#include <condition_variable>

namespace eprosima {
namespace fastdds {
namespace dds {



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

    template <typename Type, typename TypePubSubType>
    bool create_known_type(
            const std::string& type);

    void wait_discovery(
            uint32_t how_many);

    void run(
            uint32_t samples);

    void on_publication_matched(
            DataWriter* /*publisher*/,
            const PublicationMatchedStatus& info) override;

private:

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int matched_ = 0;
    bool run_ = true;
    DomainParticipant* participant_ = nullptr;

    std::map<std::string, PubKnownType> known_types_;
};

} // dds
} // fastdds
} // eprosima

#endif /* _TEST_PUBLISHER_H_ */