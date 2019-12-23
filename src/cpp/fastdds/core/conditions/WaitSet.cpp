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
 * @file WaitSet.cpp
 *
 */

#include <fastdds/dds/core/conditions/WaitSet.hpp>

using namespace eprosima;
using namespace eprosima::fastdds::dds;

ReturnCode_t WaitSet::attach_condition(
        Condition* condition)
{
    std::lock_guard<std::mutex> lock(mtx_cond_);

    auto it = std::find(attached_conditions_.begin(), attached_conditions_.end(), condition);
    if (it != attached_conditions_.end())
    {
        return ReturnCode_t::RETCODE_OK;
    }
    attached_conditions_.push_back(condition);
    condition->attached(true);
    if (condition->get_trigger_value())
    {
        cv_.notify_one();
    }
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t WaitSet::detach_condition(
        Condition* condition)
{
    std::lock_guard<std::mutex> lock(mtx_cond_);

    auto it = std::find(attached_conditions_.begin(), attached_conditions_.end(), condition);
    if (it == attached_conditions_.end())
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }
    attached_conditions_.erase(it);
    condition->attached(false);
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t WaitSet::wait(
        ConditionSeq& active_conditions,
        const fastrtps::Duration_t& timeout)
{
    if (waiting)
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    waiting = true;

    active_conditions.clear();

    std::chrono::microseconds max_wait(eprosima::fastrtps::rtps::TimeConv::Duration_t2MicroSecondsInt64(timeout));

    std::unique_lock<std::mutex> lock(mtx_cond_);

    if (cv_.wait_for(lock, max_wait, [&]()
    {
        bool unblock = false;
        for (auto cond : attached_conditions_)
        {
            if (cond->get_trigger_value())
            {
                active_conditions.push_back(cond);
                unblock = true;
            }
        }
        std::reverse(active_conditions.begin(), active_conditions.end());
        return unblock;
    }))
    {
        waiting = false;
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        waiting = false;
        return ReturnCode_t::RETCODE_TIMEOUT;
    }

}

ReturnCode_t WaitSet::dispatch(
        const fastrtps::Duration_t& timeout)
{
    if (waiting)
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    waiting = true;

    std::chrono::microseconds max_wait(eprosima::fastrtps::rtps::TimeConv::Duration_t2MicroSecondsInt64(timeout));

    std::unique_lock<std::mutex> lock(mtx_cond_);

    if (cv_.wait_for(lock, max_wait, [&]()
    {
        bool unblock = false;
        for (auto cond : attached_conditions_)
        {
            if (cond->get_trigger_value())
            {
                cond->call_handler();
                unblock = true;
            }
        }
        return unblock;
    }))
    {
        waiting = false;
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        waiting = false;
        return ReturnCode_t::RETCODE_TIMEOUT;
    }

}
