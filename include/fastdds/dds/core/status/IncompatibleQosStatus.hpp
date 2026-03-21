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

#ifndef FASTDDS_DDS_CORE_STATUS__INCOMPATIBLEQOSSTATUS_HPP
#define FASTDDS_DDS_CORE_STATUS__INCOMPATIBLEQOSSTATUS_HPP

#include <cstdint>
#include <vector>

#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief A struct storing the id of the incompatible QoS Policy and the number of times it fails
 */
struct QosPolicyCount
{
    //!Constructor
    QosPolicyCount() = default;

    //!Constructor
    QosPolicyCount(
            QosPolicyId_t id,
            int32_t c)
        : policy_id(id)
        , count(c)
    {
    }

    //! @brief The id of the policy
    QosPolicyId_t policy_id = INVALID_QOS_POLICY_ID;

    //! @brief Total number of times that the concerned writer discovered a reader for the same topic
    //! @details The requested QoS is incompatible with the one offered by the writer
    uint32_t count = 0;
};

//!Alias of std::vector<QosPolicyCount>
using QosPolicyCountSeq = std::vector<QosPolicyCount>;

//! @brief A struct storing the requested incompatible QoS status
struct IncompatibleQosStatus
{
    //! @brief Total cumulative number of times the concerned writer discovered a reader for the same topic
    //! @details The requested QoS is incompatible with the one offered by the writer
    uint32_t total_count = 0;

    //! @brief The change in total_count since the last time the listener was called or the status was read
    uint32_t total_count_change = 0;

    //! @brief The id of the policy that was found to be incompatible the last time an incompatibility is detected
    QosPolicyId_t last_policy_id = INVALID_QOS_POLICY_ID;

    //! @brief A list of QosPolicyCount
    QosPolicyCountSeq policies;

    IncompatibleQosStatus()
        : policies(static_cast<size_t>(NEXT_QOS_POLICY_ID), QosPolicyCount(INVALID_QOS_POLICY_ID, 0))
    {
        for (uint32_t id = 0; id < NEXT_QOS_POLICY_ID; ++id)
        {
            policies[id].policy_id = static_cast<QosPolicyId_t>(id);
            policies[id].count = 0;
        }
    }

};

//!Alias of IncompatibleQosStatus
using RequestedIncompatibleQosStatus = IncompatibleQosStatus;
//!Alias of IncompatibleQosStatus
using OfferedIncompatibleQosStatus = IncompatibleQosStatus;

} //end of namespace dds
} //end of namespace fastdds
} //end of namespace eprosima

#endif // FASTDDS_DDS_CORE_STATUS__INCOMPATIBLEQOSSTATUS_HPP
