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

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

ReturnCode_t WaitSetImpl::attach_condition(
        const Condition& condition)
{
    std::lock_guard<std::mutex> guard(mutex_);
    bool was_there = entries_.remove(&condition);
    entries_.emplace_back(&condition);

    // Should wake_up when adding a new triggered condition
    if (is_waiting_ && !was_there && condition.get_trigger_value())
    {
        wake_up();
    }

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t WaitSetImpl::detach_condition(
        const Condition& condition)
{
    std::lock_guard<std::mutex> guard(mutex_);
    bool was_there = entries_.remove(&condition);
    return was_there ? ReturnCode_t::RETCODE_OK : ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
}

ReturnCode_t WaitSetImpl::wait(
        ConditionSeq& active_conditions,
        const fastrtps::Duration_t& timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (is_waiting_)
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    auto fill_active_conditions = [&]()
            {
                bool ret_val = entries_.empty();
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
    if (fastrtps::c_TimeInfinite == timeout)
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

    return condition_value ? ReturnCode_t::RETCODE_OK : ReturnCode_t::RETCODE_TIMEOUT;
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
    return ReturnCode_t::RETCODE_OK;
}

void WaitSetImpl::wake_up()
{
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
