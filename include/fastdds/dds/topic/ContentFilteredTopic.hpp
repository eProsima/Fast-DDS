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
 * @file ContentFilteredTopic.hpp
 */

#ifndef _FASTDDS_DDS_TOPIC_CONTENTFILTEREDTOPIC_HPP_
#define _FASTDDS_DDS_TOPIC_CONTENTFILTEREDTOPIC_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#define FASTDDS_SQLFILTER_NAME "DDSSQL"

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;
class DomainParticipantImpl;
class ContentFilteredTopicImpl;

/**
 * Specialization of TopicDescription that allows for content-based subscriptions
 * @ingroup FASTDDS_MODULE
 */
class ContentFilteredTopic : public TopicDescription
{
    friend class ContentFilteredTopicImpl;
    friend class DomainParticipantImpl;

    RTPS_DllAPI ContentFilteredTopic(
            const std::string& name,
            Topic* related_topic,
            const std::string& filter_expression,
            const std::vector<std::string>& expression_parameters);

public:

    /**
     * @brief Destructor
     */
    RTPS_DllAPI virtual ~ContentFilteredTopic();

    /**
     * @brief Getter for the related topic.
     * This operation returns the Topic associated with the ContentFilteredTopic.
     * That is, the Topic specified when the ContentFilteredTopic was created.
     */
    RTPS_DllAPI Topic* get_related_topic() const;

    RTPS_DllAPI const std::string& get_expression() const;

    RTPS_DllAPI ReturnCode_t get_expression_parameters(
            std::vector<std::string>& expression_parameters) const;

    RTPS_DllAPI ReturnCode_t set_expression_parameters(
            const std::vector<std::string>& expression_parameters);

    RTPS_DllAPI ReturnCode_t set_expression(
            const std::string& filter_expression,
            const std::vector<std::string>& expression_parameters);

    /**
     * @brief Getter for the DomainParticipant
     * @return DomainParticipant pointer
     */
    virtual DomainParticipant* get_participant() const override;

    /**
     * @brief Getter for the TopicDescriptionImpl
     * @return pointer to TopicDescriptionImpl
     */
    TopicDescriptionImpl* get_impl() const override;

protected:

    ContentFilteredTopicImpl* impl_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _FASTDDS_DDS_TOPIC_CONTENTFILTEREDTOPIC_HPP_
