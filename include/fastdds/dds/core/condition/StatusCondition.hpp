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

namespace detail {

struct StatusConditionImpl;

} // namespace detail

class Entity;

/**
 * @brief The StatusCondition class is a specific Condition that is associated with each Entity.
 *
 */
class StatusCondition final : public Condition
{
public:

    StatusCondition(
            Entity* parent);

    ~StatusCondition() final;

    // Non-copyable
    StatusCondition(
            const StatusCondition&) = delete;
    StatusCondition& operator =(
            const StatusCondition&) = delete;

    // Non-movable
    StatusCondition(
            StatusCondition&&) = delete;
    StatusCondition& operator =(
            StatusCondition&&) = delete;

    /**
     * @brief Retrieves the trigger_value of the Condition
     * @return true if trigger_value is set to 'true', 'false' otherwise
     */
    RTPS_DllAPI bool get_trigger_value() const override;

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

    detail::StatusConditionImpl* get_impl() const
    {
        return impl_.get();
    }

protected:

    //! DDS Entity for which this condition is monitoring the status
    Entity* entity_ = nullptr;

    //! Class implementation
    std::unique_ptr<detail::StatusConditionImpl> impl_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_STATUS_CONDITION_HPP_
