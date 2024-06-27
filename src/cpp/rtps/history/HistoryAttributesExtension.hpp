// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * @file HistoryAttributesExtension.hpp
 *
 */

#ifndef FASTDDS_RTPS_HISTORY_HISTORYATTRIBUTESEXTENSION_HPP_
#define FASTDDS_RTPS_HISTORY_HISTORYATTRIBUTESEXTENSION_HPP_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

static inline ResourceLimitedContainerConfig resource_limits_from_history(
        const HistoryAttributes& history_attributes,
        size_t increment = 1u)
{
    if (history_attributes.maximumReservedCaches > 0 &&
            history_attributes.initialReservedCaches == history_attributes.maximumReservedCaches)
    {
        return ResourceLimitedContainerConfig::fixed_size_configuration(history_attributes.maximumReservedCaches);
    }

    return
        {
            history_attributes.initialReservedCaches > 0 ?
            static_cast<size_t>(history_attributes.initialReservedCaches) : 0,
            history_attributes.maximumReservedCaches > 0 ?
            static_cast<size_t>(history_attributes.maximumReservedCaches) : std::numeric_limits<size_t>::max(),
            increment > 0 ? increment : 1u
        };
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_HISTORY_HISTORYATTRIBUTESEXTENSION_HPP_
