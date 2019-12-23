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
 * @file Condition.hpp
 *
 */

#ifndef _FASTDDS_CONDITION_HPP_
#define _FASTDDS_CONDITION_HPP_

#include <fastrtps/fastrtps_all.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class RTPS_DllAPI Condition
{
public:

    bool get_trigger_value()
    {
        return trigger_value_;
    }

    virtual void call_handler(
            Condition* cond)
    {
        (void) cond;
    }

    /**
     * @brief operator ==
     * @param obj
     * @return Always false as you cannot compare the base class
     */
    virtual bool operator ==(
            Condition* obj) const
    {
        (void) obj;
        std::cout << "Condition operator" << std::endl;
        return false;
    }

protected:

    bool trigger_value_;

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif /* _FASTDDS_CONDITION_HPP_ */
