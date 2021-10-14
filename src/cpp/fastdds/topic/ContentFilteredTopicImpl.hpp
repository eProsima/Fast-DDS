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
 * @file ContentFilteredTopicImpl.hpp
 */

#ifndef _FASTDDS_TOPIC_CONTENTFILTEREDTOPICIMPL_HPP_
#define _FASTDDS_TOPIC_CONTENTFILTEREDTOPICIMPL_HPP_

#include <string>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/writer/IReaderDataFilter.hpp>

#include <fastdds/topic/TopicDescriptionImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class ContentFilteredTopicImpl : public TopicDescriptionImpl, public eprosima::fastdds::rtps::IReaderDataFilter
{
public:

    virtual ~ContentFilteredTopicImpl() = default;

    const std::string& get_rtps_topic_name() const override
    {
        return related_topic->get_name();
    }

    bool is_relevant(
            const fastrtps::rtps::CacheChange_t& change,
            const fastrtps::rtps::GUID_t& reader_guid) const override
    {
        (void)change;
        (void)reader_guid;
        return true;
    }

    Topic* related_topic = nullptr;
    std::string expression;
    std::vector<std::string> parameters;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_TOPIC_CONTENTFILTEREDTOPICIMPL_HPP_
