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
 * @file ContentFilterProperty.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DATA_CONTENTFILTERPROPERTY_HPP_
#define FASTDDS_RTPS_BUILTIN_DATA_CONTENTFILTERPROPERTY_HPP_

#include <string>

#include <fastrtps/utils/fixed_size_string.hpp>
#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class ContentFilterProperty.
 * *@ingroup BUILTIN_MODULE
 */
class ContentFilterProperty
{
public:

    struct AllocationConfiguration
    {
        size_t expression_initial_size = 0;
        fastrtps::ResourceLimitedContainerConfig expression_parameters{ 0, 100, 1 };
    };

    explicit ContentFilterProperty(
            const AllocationConfiguration& config)
        : expression_parameters(config.expression_parameters)
    {
        filter_expression.reserve(config.expression_initial_size);
    }

    fastrtps::string_255 content_filtered_topic_name;
    fastrtps::string_255 related_topic_name;
    fastrtps::string_255 filter_class_name;
    std::string filter_expression;
    fastrtps::ResourceLimitedVector<fastrtps::string_255, std::true_type> expression_parameters;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DATA_CONTENTFILTERPROPERTY_HPP_
