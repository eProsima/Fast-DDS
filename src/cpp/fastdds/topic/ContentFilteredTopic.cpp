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
 * @file ContentFilteredTopic.cpp
 *
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp>

#include <fastdds/topic/ContentFilteredTopicImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

ContentFilteredTopic::ContentFilteredTopic(
        const std::string& name,
        Topic* related_topic,
        const std::string& filter_expression,
        const std::vector<std::string>& expression_parameters)
    : TopicDescription(name, related_topic->get_type_name())
    , impl_(nullptr)
{
    related_topic->get_impl()->reference();

    impl_ = new ContentFilteredTopicImpl();
    impl_->related_topic = related_topic;
    impl_->filter_property.content_filtered_topic_name = name;
    impl_->filter_property.related_topic_name = related_topic->get_name();
    impl_->filter_property.filter_expression = filter_expression;
    impl_->filter_property.expression_parameters.assign(expression_parameters.begin(), expression_parameters.end());
}

ContentFilteredTopic::~ContentFilteredTopic()
{
    impl_->related_topic->get_impl()->dereference();
    impl_->filter_factory->delete_content_filter(
        impl_->filter_property.filter_class_name.c_str(), impl_->filter_instance);
    delete impl_;
}

Topic* ContentFilteredTopic::get_related_topic() const
{
    return impl_->related_topic;
}

const std::string& ContentFilteredTopic::get_filter_expression() const
{
    return impl_->filter_property.filter_expression;
}

ReturnCode_t ContentFilteredTopic::get_expression_parameters(
        std::vector<std::string>& expression_parameters) const
{
    expression_parameters.clear();
    expression_parameters.reserve(impl_->filter_property.expression_parameters.size());
    for (const auto& param : impl_->filter_property.expression_parameters)
    {
        expression_parameters.emplace_back(param.c_str());
    }
    return RETCODE_OK;
}

ReturnCode_t ContentFilteredTopic::set_expression_parameters(
        const std::vector<std::string>& expression_parameters)
{
    return impl_->set_expression_parameters(nullptr, expression_parameters);
}

ReturnCode_t ContentFilteredTopic::set_filter_expression(
        const std::string& filter_expression,
        const std::vector<std::string>& expression_parameters)
{
    return impl_->set_expression_parameters(filter_expression.c_str(), expression_parameters);
}

/**
 * @brief Getter for the DomainParticipant
 * @return DomainParticipant pointer
 */
DomainParticipant* ContentFilteredTopic::get_participant() const
{
    return impl_->related_topic->get_participant();
}

TopicDescriptionImpl* ContentFilteredTopic::get_impl() const
{
    return impl_;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
