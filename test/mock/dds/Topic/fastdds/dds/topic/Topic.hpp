// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Topic.hpp
 */

#ifndef FASTDDS_DDS_TOPIC__TOPIC_HPP
#define FASTDDS_DDS_TOPIC__TOPIC_HPP

#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>

#include <string>
#include <gmock/gmock.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class Topic : public TopicDescription
{
public:

    Topic(
            const std::string& topic_name,
            const std::string& type_name)
        : TopicDescription(topic_name, type_name)
    {
    }

    MOCK_CONST_METHOD0(get_participant, DomainParticipant * ());
    MOCK_CONST_METHOD0(get_impl, TopicDescriptionImpl * ());

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_TOPIC__TOPIC_HPP
