// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_RPC__REQUESTREPLYCONTENTFILTERFACTORY_HPP
#define FASTDDS_RPC__REQUESTREPLYCONTENTFILTERFACTORY_HPP

#include <fastdds/dds/topic/IContentFilterFactory.hpp>

#include "RequestReplyContentFilter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * @brief This class represents the factory used to create the filter for Request/Reply topics
 */
class RequestReplyContentFilterFactory : public IContentFilterFactory
{
public:

    constexpr static const char* FILTER_NAME = "__BUILTIN_REQUEST_REPLY_CONTENT_FILTER__";

    ReturnCode_t create_content_filter(
            const char* filter_class_name,
            const char* type_name,
            const TopicDataType* data_type,
            const char* filter_expression,
            const ParameterSeq& filter_parameters,
            IContentFilter*& filter_instance) override
    {
        static_cast<void>(type_name);
        static_cast<void>(data_type);
        static_cast<void>(filter_expression);
        static_cast<void>(filter_parameters);

        if (0 != strcmp(filter_class_name, FILTER_NAME))
        {
            return RETCODE_BAD_PARAMETER;
        }

        filter_instance = &filter_instance_;

        return RETCODE_OK;
    }

    ReturnCode_t delete_content_filter(
            const char* filter_class_name,
            IContentFilter* filter_instance) override
    {
        static_cast<void>(filter_instance);

        if (0 != strcmp(filter_class_name, FILTER_NAME))
        {
            return RETCODE_BAD_PARAMETER;
        }

        return RETCODE_OK;
    }

private:

    RequestReplyContentFilter filter_instance_;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RPC__REQUESTREPLYCONTENTFILTERFACTORY_HPP