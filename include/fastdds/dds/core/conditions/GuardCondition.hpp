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
 * @file GuardCondition.hpp
 *
 */

#ifndef _FASTDDS_DDS_GUARDCONDITION_HPP_
#define _FASTDDS_DDS_GUARDCONDITION_HPP_

#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/core/conditions/Condition.hpp>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief a specific Condition whose trigger_value is completely under the control of the application.
 */
class GuardCondition : public Condition
{
public:

    RTPS_DllAPI GuardCondition()
    {
    }

    virtual ~GuardCondition() = default;

    /**
     * @brief This operation sets the trigger_value of the condition.
     * @return true if the trigger_value is set.
     */
    RTPS_DllAPI ReturnCode_t set_trigger_value(
            bool value)
    {
        trigger_value_ = value;
        return ReturnCode_t::RETCODE_OK;
    }


};


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_GUARDCONDITION_HPP_
