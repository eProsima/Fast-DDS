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
 * @file IContentFilter.hpp
 */

#ifndef _FASTDDS_DDS_TOPIC_ICONTENTFILTERFACTORY_HPP_
#define _FASTDDS_DDS_TOPIC_ICONTENTFILTERFACTORY_HPP_

#include <fastrtps/fastrtps_dll.h>

#include <fastdds/dds/core/LoanableTypedCollection.hpp>
#include <fastdds/dds/topic/IContentFilter.hpp>

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/TypeDescriptor.h>

namespace eprosima {
namespace fastdds {
namespace dds {

struct IContentFilterFactory
{
    using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;
    using ParameterSeq = LoanableTypedCollection<const char*>;
    using TypeDescriptor = eprosima::fastrtps::types::TypeDescriptor;

    /**
     * @brief Destructor
     */
    virtual ~IContentFilterFactory() = default;

    virtual ReturnCode_t create_content_filter(
            const char* type_name,
            const TypeDescriptor* type_description,
            const char* filter_expression,
            const ParameterSeq& filter_parameters,
            IContentFilter*& filter_instance) = 0;

    virtual ReturnCode_t delete_content_filter(
            IContentFilter* filter_instance) = 0;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _FASTDDS_DDS_TOPIC_ICONTENTFILTERFACTORY_HPP_
