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
 * @file WaitSet.hpp
 *
 */

#ifndef _FASTDDS_WAITSET_HPP_
#define _FASTDDS_WAITSET_HPP_

#include <fastrtps/fastrtps_all.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/common/Time_t.h>

#include <fastdds/dds/core/conditions/Condition.hpp>

#include <mutex>
#include <condition_variable>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

using ConditionSeq = std::vector<Condition*>;

class RTPS_DllAPI WaitSet
{
public:

    ReturnCode_t attach_condition(
            Condition* condition);

    ReturnCode_t detach_condition(
            Condition* condition);

    ReturnCode_t wait(
            ConditionSeq& active_conditions,
            const fastrtps::Duration_t& timeout);

    ReturnCode_t get_conditions(
            ConditionSeq& attached_conditions)
    {
        attached_conditions = attached_conditions_;
        return ReturnCode_t::RETCODE_OK;
    }

private:

    ConditionSeq attached_conditions_;

    std::mutex mtx_cond_;

    std::condition_variable cv_;

    bool waiting = false;

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif /* _FASTDDS_WAITSET_HPP_ */
