/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef EPROSIMA_DDS_CORE_COND_TCONDITION_IMPL_HPP_
#define EPROSIMA_DDS_CORE_COND_TCONDITION_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/detail/ReferenceImpl.hpp>
#include <dds/core/cond/Condition.hpp>

namespace dds {
namespace core {
namespace cond {

template<typename DELEGATE>
TCondition<DELEGATE>::~TCondition()
{
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template<typename DELEGATE>
template<typename Functor>
void TCondition<DELEGATE>::handler(
        Functor& func)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->set_handler(func);

}

template<typename DELEGATE>
template<typename Functor>
void TCondition<DELEGATE>::handler(
        const Functor& func)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //        this->delegate()->set_handler(func);
}

/** @endcond */

template<typename DELEGATE>
void TCondition<DELEGATE>::reset_handler()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->reset_handler();
}

template<typename DELEGATE>
void TCondition<DELEGATE>::dispatch()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->dispatch();
}

template<typename DELEGATE>
bool TCondition<DELEGATE>::trigger_value() const
{
    //    return this->delegate()->get_trigger_value();
}

} //namespace cond
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_COND_TCONDITION_IMPL_HPP_
