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

class Condition
{
public:

    RTPS_DllAPI Condition()
        : is_attached_(false)
        , handler(nullptr)
    {
    }

    RTPS_DllAPI Condition(
            std::function<void()> functor)
        : is_attached_(false)
        , handler(functor)
    {
    }

    RTPS_DllAPI bool get_trigger_value()
    {
        return trigger_value_;
    }

    RTPS_DllAPI void attached(
            bool value)
    {
        is_attached_ = value;
    }

    RTPS_DllAPI bool is_attached()
    {
        return is_attached_;
    }

    /**
     * @brief set_handler Link a handler to the Condition
     * @param functor Handler that is going to be applied when the Condition is triggered
     */
    RTPS_DllAPI void set_handler(
            std::function<void()> functor)
    {
        handler = functor;
    }

    /**
     * @brief call_handler Called when the condition is triggered. It call the handler associated
     * to the Condition to manage the change in the application.
     */
    RTPS_DllAPI void call_handler()
    {
        handler();
    }

    /**
     * @brief operator ==
     * @param obj
     * @return Always false as you cannot compare the base class
     */
    RTPS_DllAPI virtual bool operator ==(
            Condition* obj) const
    {
        (void) obj;
        return false;
    }

protected:

    bool trigger_value_;

    bool is_attached_;

    //!Function handler
    std::function<void()> handler;

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif /* _FASTDDS_CONDITION_HPP_ */
