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
 * @file WaitSetImpl.hpp
 */

#ifndef _FASTDDS_CORE_CONDITION_WAITSETIMPL_HPP_
#define _FASTDDS_CORE_CONDITION_WAITSETIMPL_HPP_

#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <utils/collections/unordered_vector.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct WaitSetImpl
{
    ~WaitSetImpl();

    WaitSetImpl() = default;

    // Non-copyable
    WaitSetImpl(
            const WaitSetImpl&) = delete;
    WaitSetImpl& operator =(
            const WaitSetImpl&) = delete;

    // Non-movable
    WaitSetImpl(
            WaitSetImpl&&) = delete;
    WaitSetImpl& operator =(
            WaitSetImpl&&) = delete;

    /**
     * @brief Attach a Condition to this WaitSet implementation
     * @param condition The Condition to attach to this WaitSet implementation
     * @return RETCODE_OK
     */
    ReturnCode_t attach_condition(
            const Condition& condition);

    /**
     * @brief Detach a Condition from this WaitSet implementation
     * @param condition The Condition to detach from this WaitSet implementation
     * @return RETCODE_OK if detached correctly
     * @return PRECONDITION_NOT_MET if condition was not attached
     */
    ReturnCode_t detach_condition(
            const Condition& condition);

    /**
     * @brief Wait for any of the attached conditions to be triggered.
     * If none of the conditions attached to this WaitSet implementation have a trigger_value of true,
     * this operation will block, suspending the calling thread.
     * The list of conditions with a trigger_value of true will be returned on active_conditions.
     * It is not possible to call this operation from two different threads at the same time (PRECONDITION_NOT_MET
     * will be returned in that case)
     *
     * @param active_conditions Reference to the collection of conditions that have a trigger_value of true
     * @param timeout Maximum time of the wait
     * @return RETCODE_OK if everything correct
     * @return PRECONDITION_NOT_MET if WaitSet already waiting
     * @return TIMEOUT if maximum time expired
     */
    ReturnCode_t wait(
            ConditionSeq& active_conditions,
            const fastdds::dds::Duration_t& timeout);

    /**
     * @brief Retrieve the list of attached conditions
     * @param attached_conditions Reference to the collection of attached conditions
     * @return RETCODE_OK
     */
    ReturnCode_t get_conditions(
            ConditionSeq& attached_conditions) const;

    /**
     * @brief Wake up this WaitSet implementation if it was waiting
     */
    void wake_up();

    /**
     * @brief Called from the destructor of a Condition to inform this WaitSet implementation that the condition
     * should be automatically detached.
     */
    void will_be_deleted (
            const Condition& condition);

private:

    mutable std::mutex mutex_;
    std::condition_variable cond_;
    eprosima::utilities::collections::unordered_vector<const Condition*> entries_;
    bool is_waiting_ = false;
};

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // _FASTDDS_CORE_CONDITION_WAITSETIMPL_HPP_
