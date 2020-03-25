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
 * @file Condition.hpp
 *
 */

#ifndef _FASTDDS_DDS_CONDITION_HPP_
#define _FASTDDS_DDS_CONDITION_HPP_

#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief Is the root class for all the conditions that may be attached to a WaitSet
 *
 * This basic class is specialized in three classes that are known by the middleware:
 * GuardCondition, StatusCondition, and ReadCondition
 *
 */
class Condition
{
public:

    RTPS_DllAPI Condition()
        : trigger_value_(false)
    {
    }

    virtual ~Condition() = default;

    /**
     * @brief This operation returns the trigger_value of the condition.
     * @return true if the trigger_value is set.
     */
    RTPS_DllAPI bool get_trigger_value()
    {
        return trigger_value_;
    }

protected:

    bool trigger_value_;

};


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_CONDITION_HPP_
