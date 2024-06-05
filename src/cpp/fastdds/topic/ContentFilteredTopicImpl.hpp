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

#include <array>
#include <cstdint>
#include <string>

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>

#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/topic/TopicDescriptionImpl.hpp>

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
            const fastdds::rtps::CacheChange_t& change,
            const fastdds::rtps::GUID_t& reader_guid) const final;

    /**
     * Add an entry to the list of DataReaderImpl that should be notified of changes to this object.
     *
     * @param [in] reader  Pointer to the DataReaderImpl to add.
     */
    void add_reader(
            DataReaderImpl* reader)
    {
        readers_.insert(reader);
    }

    /**
     * Remove an entry from the list of DataReaderImpl that should be notified of changes to this object.
     *
     * @param [in] reader  Pointer to the DataReaderImpl to remove.
     */
    void remove_reader(
            DataReaderImpl* reader)
    {
        readers_.erase(reader);
    }

    /**
     * Compute signature values from the current configuration of this object.
     */
    void update_signature();

    /**
     * Set new filtering configuration for this object.
     * The filter signatures will be accordingly updated.
     * The registered readers will be then notified.
     *
     * @param [in]  new_expression             New value to be set for the expression.
     *                                         May be nullptr to indicate that only the parameters have been changed.
     *                                         May be an empty string to indicate that filtering should be disabled.
     * @param [in]  new_expression_parameters  New value for the expression parameters.
     *
     * @return a code indicating if this object was correctly updated.
     */
    ReturnCode_t set_expression_parameters(
            const char* new_expression,
            const std::vector<std::string>& new_expression_parameters);

    IContentFilterFactory* filter_factory = nullptr;
    IContentFilter* filter_instance = nullptr;
    Topic* related_topic = nullptr;
    eprosima::fastdds::rtps::ContentFilterProperty filter_property{{}};

private:

    /**
     * Inspects the inline QoS of a change to check if the writer already applied this filter.
     *
     * @param [in]  change         The change for which the inline QoS should be checked.
     * @param [out] filter_result  When the signature of this filter is present on the inline QoS,
     *                             this will be set with the filter evaluation done by the writer.
     *                             Only valid when the method returns true.
     *
     * @return whether the signature for the current filter was found on the inline QoS of the change.
     */
    bool check_filter_signature(
            const fastdds::rtps::CacheChange_t& change,
            bool& filter_result) const;

    std::set<DataReaderImpl*> readers_;
    std::array<uint8_t, 16> filter_signature_;
    std::array<uint8_t, 16> filter_signature_rti_connext_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_TOPIC_CONTENTFILTEREDTOPICIMPL_HPP_
