// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file GuardCondition.hpp
 *
 */

#ifndef _FASTDDS_DDS_STATUSCONDITION_HPP_
#define _FASTDDS_DDS_STATUSCONDITION_HPP_

#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/core/conditions/Condition.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

class Entity;

/**
 * @brief a specific Condition that is associated with an Entity.
 */
class StatusCondition : public Condition
{

    friend class Entity;

public:

    virtual ~StatusCondition() = default;

    /**
     * @brief This operation defines the list of statuses that are taken into account
     * to determine the trigger_value
     *
     * This operation may change the trigger_value of the StatusCondition depending on the
     * trigger state of the new statuses.
     *
     * WaitSet objects behavior depend on the changes of the trigger_value of their attached
     * conditions. Therefore, any WaitSet to which the StatusCondition is attached is
     * potentially affected by this operation.
     *
     * If this function is not invoked, the default list of enabled statuses includes all the statuses.
     *
     * @return true if the statuses are set.
     */
    RTPS_DllAPI ReturnCode_t set_enabled_statuses(
            StatusMask mask);

    /**
     * @brief This operation retrieves the list of statuses that are taken into account
     * to determine the trigger_value
     *
     * @return reference to the list of statuses taken into account in the trigger_value
     */
    RTPS_DllAPI const StatusMask& get_enabled_statuses()
    {
        return enabled_statuses_;
    }

    /**
     * @brief returns the Entity associated with the StatusCondition.
     *
     * @return reference to the Entity associated with the StatusCondition.
     */
    RTPS_DllAPI Entity& get_entity()
    {
        return *entity_;
    }


protected:

    StatusCondition(
            Entity* entity)
    : enabled_statuses_(StatusMask::all())
    , entity_(entity)
    {
    }

    StatusMask enabled_statuses_;

    Entity* entity_;

};


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_STATUSCONDITION_HPP_
