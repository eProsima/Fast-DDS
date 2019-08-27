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

#ifndef EPROSIMA_DDS_CORE_COND_TWAITSET_HPP_
#define EPROSIMA_DDS_CORE_COND_TWAITSET_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/cond/WaitSet.hpp>
//TODO: Fix when ReportUtils is implemented
//#include <org/opensplice/core/ReportUtils.hpp>

namespace dds {
namespace core {
namespace cond {

template<typename DELEGATE>
TWaitSet<DELEGATE>::TWaitSet()
{
    //To implement
}

template<typename DELEGATE>
TWaitSet<DELEGATE>::~TWaitSet() { }

template<typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq TWaitSet<DELEGATE>::wait(
        const dds::core::Duration& timeout)
{
    //To implement
}

template<typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq TWaitSet<DELEGATE>::wait()
{
    //To implement
}

template<typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::wait(
        ConditionSeq& triggered,
        const dds::core::Duration& timeout)
{
    //To implement
}

template<typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::wait(
        ConditionSeq& triggered)
{
    //To implement
}

template<typename DELEGATE>
void TWaitSet<DELEGATE>::dispatch()
{
    //To implement
}

template<typename DELEGATE>
void TWaitSet<DELEGATE>::dispatch(
        const dds::core::Duration& timeout)
{
    //To implement
}

template<typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::operator +=(
        const Condition& cond)
{
    //To implement
}

template<typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::operator -=(
        const Condition& cond)
{
    //To implement
}

template<typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::attach_condition(
        const Condition& cond)
{
    //To implement
}

template<typename DELEGATE>
bool TWaitSet<DELEGATE>::detach_condition(
        const Condition& cond)
{
    //To implement
}

template<typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq TWaitSet<DELEGATE>::conditions() const
{
    //To implement
}

template<typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::conditions(
        ConditionSeq& conds) const
{
    //To implement
}

} //namespace cond
} //namespace core
} //namespace dds


#endif //EPROSIMA_DDS_CORE_COND_TWAITSET_HPP_
