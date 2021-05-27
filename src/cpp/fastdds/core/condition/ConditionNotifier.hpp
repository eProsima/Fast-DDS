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
 * @file ConditionNotifier.hpp
 */

#ifndef _FASTDDS_CORE_CONDITION_CONDITIONNOTIFIER_HPP_
#define _FASTDDS_CORE_CONDITION_CONDITIONNOTIFIER_HPP_

#include <mutex>

#include <fastdds/dds/core/condition/Condition.hpp>

#include <utils/collections/unordered_vector.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct WaitSetImpl;

struct ConditionNotifier
{
    /**
     * Add a WaitSet implementation to the list of attached entries.
     * Does nothing if wait_set was already attached to this notifier.
     * @param wait_set WaitSet implementation to add to the list.
     */
    void attach_to (
            WaitSetImpl* wait_set);


    /**
     * Remove a WaitSet implementation from the list of attached entries.
     * Does nothing if wait_set was not attached to this notifier.
     * @param wait_set WaitSet implementation to remove from the list.
     */
    void detach_from (
            WaitSetImpl* wait_set);

    /**
     * Wake up all the WaitSet implementations attached to this notifier.
     */
    void notify ();

    /**
     * Inform all the WaitSet implementations attached to this notifier that
     * a condition is going to be deleted.
     * @param condition The Condition being deleted.
     */
    void will_be_deleted (
            const Condition& condition);

private:

    std::mutex mutex_;
    eprosima::utilities::collections::unordered_vector<WaitSetImpl*> entries_;
};

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // _FASTDDS_CORE_CONDITION_CONDITIONNOTIFIER_HPP_
