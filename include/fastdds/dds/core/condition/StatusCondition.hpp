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
 * @file StatusCondition.hpp
 *
 */

#ifndef _FASTDDS_STATUS_CONDITION_HPP_
#define _FASTDDS_STATUS_CONDITION_HPP_

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

class Entity;

/**
 * @brief The StatusCondition class is a specific Condition that is associated with each Entity.
 *
 */
class StatusCondition : public Condition
{
public:

    // StatusCondition not implemented.

    /**
     * @brief Defines the list of communication statuses that are taken into account to determine the trigger_value
     * @param mask defines the mask for the status
     * @return RETCODE_OK with everything ok, error code otherwise
     */
    RTPS_DllAPI ReturnCode_t set_enabled_statuses(
            const StatusMask& mask);

    /**
     * @brief Retrieves the list of communication statuses that are taken into account to determine the trigger_value
     * @return Status set or default status if it has not been set
     */
    RTPS_DllAPI const StatusMask& get_enabled_statuses() const;

    /**
     * @brief Returns the Entity associated
     * @return Entity
     */
    RTPS_DllAPI Entity* get_entity() const;

protected:

    //! StatusMask with relevant statuses set to 1
    StatusMask status_mask;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_STATUS_CONDITION_HPP_
