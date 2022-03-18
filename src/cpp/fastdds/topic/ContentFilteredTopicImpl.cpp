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

#include <algorithm>

#include <fastrtps/utils/md5.h>

#include <fastdds/subscriber/DataReaderImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

bool ContentFilteredTopicImpl::is_relevant(
        const fastrtps::rtps::CacheChange_t& change,
        const fastrtps::rtps::GUID_t& reader_guid) const
{
    bool ret_val = true;

    if (!check_filter_signature(change, ret_val))
    {
        IContentFilter::FilterSampleInfo filter_info
        {
            change.write_params.sample_identity(),
            change.write_params.related_sample_identity()
        };
        ret_val = filter_instance->evaluate(change.serializedPayload, filter_info, reader_guid);
    }

    return ret_val;
}

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

        // Update filter signature
        update_signature();

        // Inform data readers
        for (DataReaderImpl* reader : readers_)
        {
            reader->filter_has_been_updated();
        }
    }

    return ret;
}

void ContentFilteredTopicImpl::update_signature()
{
    MD5 md5_rtps;
    MD5 md5_connext;

    md5_rtps.init();
    md5_connext.init();
    // Add content_filtered_topic_name
    {
        const char* str = filter_property.content_filtered_topic_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        md5_connext.update(str, slen);
    }
    // Add related_topic_name
    {
        const char* str = filter_property.related_topic_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        md5_connext.update(str, slen);
    }
    // Add filter_class_name
    {
        const char* str = filter_property.filter_class_name.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        md5_connext.update(str, slen);
    }
    // Add filter_expression
    size_t n_params = filter_property.expression_parameters.size();
    {
        const char* str = filter_property.filter_expression.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        md5_connext.update(str, (0 == n_params) ? slen - 1 : slen);
    }
    // Add expression_parameters
    size_t i = 0;
    for (const auto& param : filter_property.expression_parameters)
    {
        const char* str = param.c_str();
        MD5::size_type slen = static_cast<MD5::size_type>(strlen(str) + 1);
        md5_rtps.update(str, slen);
        ++i;
        md5_connext.update(str, i == n_params ? slen - 1 : slen);
    }
    md5_rtps.finalize();
    md5_connext.finalize();

    std::copy_n(md5_rtps.digest, filter_signature_.size(), filter_signature_.begin());
    std::copy_n(md5_connext.digest, filter_signature_rti_connext_.size(), filter_signature_rti_connext_.begin());
}

bool ContentFilteredTopicImpl::check_filter_signature(
        const fastrtps::rtps::CacheChange_t& change,
        bool& filter_result) const
{
    static_cast<void>(change);
    static_cast<void>(filter_result);

    return false;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
