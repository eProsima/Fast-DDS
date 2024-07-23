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
 * @file WaitSetImpl.cpp
 */

#include "WaitSetImpl.hpp"

#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

#include <fastdds/core/condition/ConditionNotifier.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

WaitSetImpl::~WaitSetImpl()
{
    eprosima::utilities::collections::unordered_vector<const Condition*> old_entries;

    {
        // We only need to protect access to the collection.
        std::lock_guard<std::mutex> guard(mutex_);
        old_entries = entries_;
        entries_.clear();
    }

    for (const Condition* c : old_entries)
    {
        c->get_notifier()->detach_from(this);
    }
}

ReturnCode_t WaitSetImpl::attach_condition(
        const Condition& condition)
{
    bool was_there = false;

    {
        // We only need to protect access to the collection.
        std::lock_guard<std::mutex> guard(mutex_);

        was_there = entries_.remove(&condition);
        entries_.emplace_back(&condition);
    }

    if (!was_there)
    {
        // This is a new condition. Inform the notifier of our interest.
        condition.get_notifier()->attach_to(this);

        {
            // Might happen that a wait changes is_waiting_'s status. Protect it.
            std::lock_guard<std::mutex> guard(mutex_);

            // Should wake_up when adding a new triggered condition
            if (is_waiting_ && condition.get_trigger_value())
            {
                cond_.notify_one();
            }
        }
    }

    return RETCODE_OK;
}

ReturnCode_t WaitSetImpl::detach_condition(
        const Condition& condition)
{
    bool was_there = false;

    {
        // We only need to protect access to the collection.
        std::lock_guard<std::mutex> guard(mutex_);
        was_there = entries_.remove(&condition);
    }

    if (was_there)
    {
        // Inform the notifier we are not interested anymore.
        condition.get_notifier()->detach_from(this);
        return RETCODE_OK;
    }

    // Condition not found
    return RETCODE_PRECONDITION_NOT_MET;
}

ReturnCode_t WaitSetImpl::wait(
        ConditionSeq& active_conditions,
        const fastdds::dds::Duration_t& timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (is_waiting_)
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    auto fill_active_conditions = [&]()
            {
                bool ret_val = false;
                active_conditions.clear();
                for (const Condition* c : entries_)
                {
                    if (c->get_trigger_value())
                    {
                        ret_val = true;
                        active_conditions.push_back(const_cast<Condition*>(c));
                    }
                }
                return ret_val;
            };

    bool condition_value = false;
    is_waiting_ = true;
    if (fastdds::dds::c_TimeInfinite == timeout)
    {
        cond_.wait(lock, fill_active_conditions);
        condition_value = true;
    }
    else
    {
        auto ns = timeout.to_ns();
        condition_value = cond_.wait_for(lock, std::chrono::nanoseconds(ns), fill_active_conditions);
    }
    is_waiting_ = false;

    return condition_value ? RETCODE_OK : RETCODE_TIMEOUT;
}

ReturnCode_t WaitSetImpl::get_conditions(
        ConditionSeq& attached_conditions) const
{
    std::lock_guard<std::mutex> guard(mutex_);
    attached_conditions.reserve(entries_.size());
    attached_conditions.clear();
    for (const Condition* c : entries_)
    {
        attached_conditions.push_back(const_cast<Condition*>(c));
    }
    return RETCODE_OK;
}

void WaitSetImpl::wake_up()
{
    std::lock_guard<std::mutex> guard(mutex_);
    cond_.notify_one();
}

void WaitSetImpl::will_be_deleted (
        const Condition& condition)
{
    std::lock_guard<std::mutex> guard(mutex_);
    entries_.remove(&condition);
}

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
