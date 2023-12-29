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
 * @file StatusConditionImpl.hpp
 */

#ifndef _FASTDDS_CORE_CONDITION_STATUSCONDITIONIMPL_HPP_
#define _FASTDDS_CORE_CONDITION_STATUSCONDITIONIMPL_HPP_

#include <mutex>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>

#include <fastdds/core/condition/ConditionNotifier.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct StatusConditionImpl
{
    /**
     * Construct a StatusConditionImpl object.
     * @param notifier @ref ConditionNotifier attatched to this object.
     */
    StatusConditionImpl(
            ConditionNotifier* notifier);

    ~StatusConditionImpl();

    // Non-copyable
    StatusConditionImpl(
            const StatusConditionImpl&) = delete;
    StatusConditionImpl& operator =(
            const StatusConditionImpl&) = delete;

    // Non-movable
    StatusConditionImpl(
            StatusConditionImpl&&) = delete;
    StatusConditionImpl& operator =(
            StatusConditionImpl&&) = delete;

    /**
     * @brief Retrieves the trigger_value of the Condition
     * @return true if trigger_value is set to 'true', 'false' otherwise
     */
    bool get_trigger_value() const;

    /**
     * @brief Defines the list of communication statuses that are taken into account to determine the trigger_value
     * @param mask defines the mask for the status
     * @return RETCODE_OK with everything ok, error code otherwise
     */
    ReturnCode_t set_enabled_statuses(
            const StatusMask& mask);

    /**
     * @brief Retrieves the list of communication statuses that are taken into account to determine the trigger_value
     * @return Status set or default status if it has not been set
     */
    const StatusMask& get_enabled_statuses() const;

    /**
     * @brief Retrieves the list of communication statuses that are currently triggered.
     * @return Triggered status.
     */
    const StatusMask& get_raw_status() const
    {
        return status_;
    }

    /**
     * @brief Set the trigger value of a specific status
     * @param status The status for which to change the trigger value
     * @param trigger_value Whether the specified status should be set as triggered or non-triggered
     */
    void set_status(
            const StatusMask& status,
            bool trigger_value);

private:

    mutable std::mutex mutex_;
    StatusMask mask_{};
    StatusMask status_{};
    ConditionNotifier* notifier_;
};

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // _FASTDDS_CORE_CONDITION_STATUSCONDITIONIMPL_HPP_
