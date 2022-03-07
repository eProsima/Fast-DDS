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
 * @file ContentFilteredTopicImpl.cpp
 */

#include "ContentFilteredTopicImpl.hpp"

#include <fastdds/subscriber/DataReaderImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

ReturnCode_t ContentFilteredTopicImpl::set_expression_parameters(
        const char* new_expression,
        const std::vector<std::string>& new_expression_parameters)
{
    TopicImpl* topic_impl = dynamic_cast<TopicImpl*>(related_topic->get_impl());
    assert(nullptr != topic_impl);
    const TypeSupport& type = topic_impl->get_type();

    LoanableSequence<const char*>::size_type n_params;
    n_params = static_cast<LoanableSequence<const char*>::size_type>(new_expression_parameters.size());
    LoanableSequence<const char*> filter_parameters(n_params);
    filter_parameters.length(n_params);
    while (n_params > 0)
    {
        n_params--;
        filter_parameters[n_params] = new_expression_parameters[n_params].c_str();
    }

    ReturnCode_t ret = filter_factory->create_content_filter(
        filter_property.filter_class_name.c_str(),
        related_topic->get_type_name().c_str(),
        type.get(), new_expression, filter_parameters, filter_instance);

    if (ReturnCode_t::RETCODE_OK == ret)
    {
        filter_property.expression_parameters.assign(new_expression_parameters.begin(),
                new_expression_parameters.end());
        if (nullptr != new_expression)
        {
            filter_property.filter_expression = new_expression;
        }

        // TODO(Miguel C): update filter hash

        // Inform data readers
        for (DataReaderImpl* reader : readers_)
        {
            reader->filter_has_been_updated();
        }
    }

    return ret;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
