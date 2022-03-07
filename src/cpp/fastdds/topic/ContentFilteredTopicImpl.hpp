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

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>

#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/topic/TopicDescriptionImpl.hpp>
#include <fastdds/topic/TopicImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class ContentFilteredTopicImpl final : public TopicDescriptionImpl, public eprosima::fastdds::rtps::IReaderDataFilter
{
public:

    virtual ~ContentFilteredTopicImpl() final = default;

    const std::string& get_rtps_topic_name() const final
    {
        return related_topic->get_name();
    }

    bool is_relevant(
            const fastrtps::rtps::CacheChange_t& change,
            const fastrtps::rtps::GUID_t& reader_guid) const final
    {
        IContentFilter::FilterSampleInfo filter_info
        {
            change.write_params.sample_identity(),
            change.write_params.related_sample_identity()
        };
        return filter_instance->evaluate(change.serializedPayload, filter_info, reader_guid);
    }

    void add_reader(
            DataReaderImpl* reader)
    {
        readers_.insert(reader);
    }

    void remove_reader(
            DataReaderImpl* reader)
    {
        readers_.erase(reader);
    }

    ReturnCode_t set_expression_parameters(
            const char* new_expression,
            const std::vector<std::string>& new_expression_parameters);

    IContentFilterFactory* filter_factory = nullptr;
    IContentFilter* filter_instance = nullptr;
    Topic* related_topic = nullptr;
    eprosima::fastdds::rtps::ContentFilterProperty filter_property{{}};

private:

    std::set<DataReaderImpl*> readers_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_TOPIC_CONTENTFILTEREDTOPICIMPL_HPP_
