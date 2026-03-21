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

#ifndef FASTDDS_RTPS_BUILTIN_DATA__CONTENTFILTERPROPERTY_HPP
#define FASTDDS_RTPS_BUILTIN_DATA__CONTENTFILTERPROPERTY_HPP

#include <string>

#include <fastcdr/cdr/fixed_size_string.hpp>
#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Information about the content filter being applied by a reader.
 */
class ContentFilterProperty
{
public:

    /**
     * Allocation configuration for a ContentFilterProperty.
     */
    struct AllocationConfiguration
    {
        /// Preallocated size of the filter expression
        size_t expression_initial_size = 0;
        /// Allocation configuration for the list of expression parameters
        fastdds::ResourceLimitedContainerConfig expression_parameters{ 0, 100, 1 };
    };

    /**
     * Construct a ContentFilterProperty.
     *
     * @param config  Allocation configuration for the new object.
     */
    explicit ContentFilterProperty(
            const AllocationConfiguration& config)
        : expression_parameters(config.expression_parameters)
    {
        filter_expression.reserve(config.expression_initial_size);
    }

    /// Name of the content filtered topic on which the reader was created
    fastcdr::string_255 content_filtered_topic_name;
    /// Name of the related topic being filtered
    fastcdr::string_255 related_topic_name;
    /// Class name of the filter being used.
    /// May be empty to indicate the ContentFilterProperty is not present.
    fastcdr::string_255 filter_class_name;
    /// Filter expression indicating which content the reader wants to receive.
    /// May be empty to indicate the ContentFilterProperty is not present.
    std::string filter_expression;
    /// List of values for the parameters present on the filter expression
    fastdds::ResourceLimitedVector<fastcdr::string_255, std::true_type> expression_parameters;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DATA__CONTENTFILTERPROPERTY_HPP
