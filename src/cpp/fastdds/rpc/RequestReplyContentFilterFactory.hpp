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

    ReturnCode_t create_content_filter(
            const char* /*filter_class_name*/,
            const char* /*type_name*/,
            const TopicDataType* /*data_type*/,
            const char* /*filter_expression*/,
            const ParameterSeq& /*filter_parameters*/,
            IContentFilter*& /*filter_instance*/) override
    {
        return RETCODE_UNSUPPORTED;
    }

    ReturnCode_t delete_content_filter(
            const char* /*filter_class_name*/,
            IContentFilter* /*filter_instance*/) override
    {
        return RETCODE_UNSUPPORTED;
    }

private:

    RequestReplyContentFilter filter_instance_;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RPC__REQUESTREPLYCONTENTFILTERFACTORY_HPP
