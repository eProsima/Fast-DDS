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
 * @file WaitSet.hpp
 *
 */

#ifndef _FASTDDS_DDS_WAITSET_HPP_
#define _FASTDDS_DDS_WAITSET_HPP_

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/dds/core/conditions/Condition.hpp>

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {


using ConditionSeq = std::vector<std::shared_ptr<Condition>>;
/**
 * @brief Allows an application to wait until any of the attached Condition objects are satisfied.
 *
 * A WaitSet can be used by the application to wati until one or more of the attached Condition objects
 * has a trigger_value of TRUE or else until the timeout expires.
 *
 */
class WaitSet
{
public:

    RTPS_DllAPI WaitSet()
        : waiting_(false)
    {
    }

    /**
     * @brief Attaches a Condition to the WaitSet.

     * @return true if the Condition is attached or was already attached.
     */
    RTPS_DllAPI ReturnCode_t attach_condition(
            std::shared_ptr<Condition> a_condition)
    {
        std::lock_guard<std::mutex> guard(mtx_);
        auto it = std::find(conditions_.begin(), conditions_.end(), a_condition);
        if (it != conditions_.end())
        {
            //Already attached, nothing to do, silently ignore
            return ReturnCode_t::RETCODE_OK;
        }

        //Attach and check if we need to wake
        conditions_.push_back(a_condition);
        if (a_condition->get_trigger_value())
        {
            cv_.notify_one();
        }
        return ReturnCode_t::RETCODE_OK;
    }

    /**
     * @brief Detaches a Condition from the WaitSet.

     * @return true if the Condition was dettached.
     */
    RTPS_DllAPI ReturnCode_t detach_condition(
            std::shared_ptr<Condition> a_condition)
    {
        std::lock_guard<std::mutex> guard(mtx_);

        auto it = std::find(conditions_.begin(), conditions_.end(), a_condition);
        if (it == conditions_.end())
        {
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }

        conditions_.erase(it);
        return ReturnCode_t::RETCODE_OK;
    }

    /**
     * @brief This operation returns the trigger_value of the condition.
     * @return true if the trigger_value is set.
     */
    RTPS_DllAPI ReturnCode_t wait(
            ConditionSeq& active_conditions,
            fastrtps::Duration_t timeout)
    {
        if (waiting_)
        {
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }

        std::chrono::microseconds max_wait(eprosima::fastrtps::rtps::TimeConv::Duration_t2MicroSecondsInt64(timeout));;
        std::unique_lock<std::mutex> guard(mtx_);
        waiting_ = true;
        active_conditions.clear();

        if (cv_.wait_for(guard, max_wait, [&]()
                {
                    bool unblock = false;
                    for (auto cond : conditions_)
                    {
                        if (cond->get_trigger_value())
                        {
                            active_conditions.push_back(cond);
                            unblock = true;
                        }
                    }
                    return unblock;
                }))
        {
            waiting_ = false;
            return ReturnCode_t::RETCODE_OK;
        }
        else
        {
            waiting_ = false;
            return ReturnCode_t::RETCODE_TIMEOUT;
        }
    }

    /**
     * @brief This operation returns the trigger_value of the condition.
     * @return true if the trigger_value is set.
     */
    RTPS_DllAPI ReturnCode_t get_conditions(
            ConditionSeq& attached_conditions)
    {
        attached_conditions = conditions_;
        return ReturnCode_t::RETCODE_OK;
    }

protected:
    ConditionSeq conditions_;

    std::mutex mtx_;

    std::condition_variable cv_;

    bool waiting_;
};


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_WAITSET_HPP_
