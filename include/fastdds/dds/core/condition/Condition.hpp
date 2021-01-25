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
 * @file Condition.hpp
 *
 */

#ifndef _FASTDDS_CONDITION_HPP_
#define _FASTDDS_CONDITION_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <vector>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * @brief The Condition class is the root base class for all the conditions that may be attached to a WaitSet.
 */
class Condition
{
public:

    // Condition class not implemented.

    /**
     * @brief Retrieves the trigger_value of the Condition
     * @return true if trigger_value is set to 'true', 'false' otherwise
     */
    RTPS_DllAPI bool get_trigger_value() const
    {
        logWarning(CONDITION, "get_trigger_value public member function not implemented");
        return false; // TODO return trigger value
    }

};

typedef std::vector<Condition> ConditionSeq;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_CONDITION_HPP_
