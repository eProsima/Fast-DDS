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
 * @file DDSFilterFactory.cpp
 */

#include "DDSFilterFactory.hpp"

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

IContentFilterFactory::ReturnCode_t DDSFilterFactory::create_content_filter(
        const char* filter_class_name,
        const char* type_name,
        const TopicDataType* data_type,
        const char* filter_expression,
        const IContentFilterFactory::ParameterSeq& filter_parameters,
        IContentFilter*& filter_instance)
{
    static_cast<void>(filter_class_name);
    static_cast<void>(type_name);
    static_cast<void>(data_type);
    static_cast<void>(filter_expression);
    static_cast<void>(filter_parameters);
    static_cast<void>(filter_instance);

    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

IContentFilterFactory::ReturnCode_t DDSFilterFactory::delete_content_filter(
        const char* filter_class_name,
        IContentFilter* filter_instance)
{
    static_cast<void>(filter_class_name);
    static_cast<void>(filter_instance);

    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
