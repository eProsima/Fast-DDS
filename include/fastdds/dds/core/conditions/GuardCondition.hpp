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
 * @file GuardCondition.hpp
 *
 */

#ifndef _FASTDDS_GUARDCONDITION_HPP_
#define _FASTDDS_GUARDCONDITION_HPP_

#include <fastrtps/fastrtps_all.h>
#include <fastdds/dds/core/conditions/Condition.hpp>
#include <fastrtps/rtps/common/Types.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief The GuardCondition class allow the application to manually wakeup a WaitSet.
 * This is accomplished by attaching a GuarCondition to the WaitSet and then setting the trigger_value by
 * means of the set_trigger_value operation.
 */
class RTPS_DllAPI GuardCondition : public Condition
{
public:

    GuardCondition()
    {
        trigger_value_ = false;
    }

    ReturnCode_t set_trigger_value(
            bool value)
    {
        trigger_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }

    /**
     * @brief operator ==
     * @param obj
     * @return Always true
     */
    inline bool operator ==(
            GuardCondition* obj) const
    {
        (void) obj;
        //The purpose of this class is waking up the WaitSet so there cannot be two of them.
        return true;
    }

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif /* _FASTDDS_GUARDCONDITION_HPP_ */
