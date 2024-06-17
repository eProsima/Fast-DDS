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

#ifndef FASTDDS_DDS_TOPIC__CONTENTFILTEREDTOPIC_HPP
#define FASTDDS_DDS_TOPIC__CONTENTFILTEREDTOPIC_HPP

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/fastdds_dll.hpp>

#define FASTDDS_SQLFILTER_NAME eprosima::fastdds::dds::sqlfilter_name

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;
class DomainParticipantImpl;
class ContentFilteredTopicImpl;

constexpr const char* const sqlfilter_name = "DDSSQL";

/**
 * Specialization of TopicDescription that allows for content-based subscriptions.
 *
 * @ingroup FASTDDS_MODULE
 */
class ContentFilteredTopic : public TopicDescription
{
    friend class DomainParticipantImpl;

private:

    ContentFilteredTopic(
            const std::string& name,
            Topic* related_topic,
            const std::string& filter_expression,
            const std::vector<std::string>& expression_parameters);

public:

    virtual ~ContentFilteredTopic();

    /**
     * @brief Getter for the related topic.
     *
     * This operation returns the Topic associated with the ContentFilteredTopic.
     * That is, the Topic specified when the ContentFilteredTopic was created.
     */
    FASTDDS_EXPORTED_API Topic* get_related_topic() const;

    /**
     * @brief Get the filter expression.
     *
     * This operation returns filter expression associated with this ContentFilteredTopic.
     * It will return the @c filter_expression specified on the last successful call to @c set_expression or,
     * if that method is never called, the expression specified when the ContentFilteredTopic was created.
     *
     * @return the @c filter_expression.
     */
    FASTDDS_EXPORTED_API const std::string& get_filter_expression() const;

    /**
     * @brief Get the expression parameters.
     *
     * This operation returns expression parameters associated with this ContentFilteredTopic.
     * These will be the @c expression_parameters specified on the last successful call to @c set_expression or
     * @c set_expression_parameters.
     * If those methods have never been called, the expression parameters specified when the ContentFilteredTopic
     * was created will be returned.
     *
     * @param [out] expression_parameters  The expression parameters currently associated with the
     *                                     ContentFilteredTopic.
     *
     * @return RETCODE_OK
     */
    FASTDDS_EXPORTED_API ReturnCode_t get_expression_parameters(
            std::vector<std::string>& expression_parameters) const;

    /**
     * @brief Set the expression parameters.
     *
     * This operation changes expression parameters associated with this ContentFilteredTopic.
     *
     * @param [in] expression_parameters  The expression parameters to set.
     *
     * @return RETCODE_OK             if the expression parameters where correctly updated.
     * @return RETCODE_BAD_PARAMETER  if the expression parameters do not match with the current @c filter_expression.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_expression_parameters(
            const std::vector<std::string>& expression_parameters);

    /**
     * @brief Set the filter expression and the expression parameters.
     *
     * This operation changes the filter expression and the expression parameters associated with this
     * ContentFilteredTopic.
     *
     * @param [in] filter_expression      The filter expression to set.
     * @param [in] expression_parameters  The expression parameters to set.
     *
     * @return RETCODE_OK             if the expression and parameters where correctly updated.
     * @return RETCODE_BAD_PARAMETER  if @c filter_expression is not valid for this ContentFilteredTopic.
     * @return RETCODE_BAD_PARAMETER  if the expression parameters do not match with the @c filter_expression.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_filter_expression(
            const std::string& filter_expression,
            const std::vector<std::string>& expression_parameters);

    /**
     * @brief Getter for the DomainParticipant
     * @return DomainParticipant pointer
     */
    FASTDDS_EXPORTED_API DomainParticipant* get_participant() const override;

    TopicDescriptionImpl* get_impl() const override;

protected:

    ContentFilteredTopicImpl* impl_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_DDS_TOPIC__CONTENTFILTEREDTOPIC_HPP
