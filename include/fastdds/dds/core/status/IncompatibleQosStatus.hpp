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

#ifndef _FASTRTPS_INCOMPATIBLE_QOS_STATUS_HPP_
#define _FASTRTPS_INCOMPATIBLE_QOS_STATUS_HPP_

#include <cstdint>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace dds {

using QosPolicyId_t = uint32_t;

struct QosPolicyCount
{
    QosPolicyCount(
            QosPolicyId_t id,
            int32_t c)
        : policy_id(id)
        , count(c)
    {
    }

    //! @brief The id of the policy
    QosPolicyId_t policy_id;

    //! @brief Total number of times that the concerned writer discovered a reader for the same topic
    //! @details The requested QoS is incompatible with the one offered by the writer
    int32_t count;
};

using QosPolicyCountSeq = std::vector<QosPolicyCount>;

//! @brief A struct storing the requested incompatible QoS status
struct IncompatibleQosStatus
{
    //! @brief Total cumulative number of times the concerned writer discovered a reader for the same topic
    //! @details The requested QoS is incompatible with the one offered by the writer
    int32_t total_count = 0;

    //! @brief The change in total_count since the last time the listener was called or the status was read
    int32_t total_count_change = 0;

    //! @brief The id of the policy that was found to be incompatible the last time an incompatibility is detected
    QosPolicyId_t last_policy_id;

    //! @brief A list of QosPolicyCount
    QosPolicyCountSeq policies;
};

using RequestedIncompatibleQosStatus = IncompatibleQosStatus;
using OfferedIncompatibleQosStatus = IncompatibleQosStatus;

} //end of namespace dds
} //end of namespace fastdds
} //end of namespace eprosima

#endif // _FASTRTPS_INCOMPATIBLE_QOS_STATUS_HPP_
