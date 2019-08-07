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

/**
 * @file IncompatibleQosStatus.hpp
*/

#ifndef _FASTDDS_INCOMPATIBLE_QOS_STATUS_HPP_
#define _FASTDDS_INCOMPATIBLE_QOS_STATUS_HPP_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief A struct storing the requested incompatible QoS status
struct IncompatibleQosStatus
{
    /**
     * Total cumulative number of times the concerned endpoint
     * discovered another endpoint for the same Topic with a requested QoS that
     * is incompatible with that offered by the concerned endpoint.
     */
    uint32_t total_count = 0;

    /**
     * The change in total_count since the last time the listener was called or the status was read.
     */
    uint32_t total_count_change = 0;

    // TODO QosPolicyId_t and QosPolicyCountSeq
};

typedef IncompatibleQosStatus RequestedIncompatibleQosStatus;
typedef IncompatibleQosStatus OfferedIncompatibleQosStatus;

} //end of namespace dds
} //end of namespace fastdds
} //end of namespace eprosima

#endif // _FASTRTPS_INCOMPATIBLE_QOS_STATUS_HPP_
