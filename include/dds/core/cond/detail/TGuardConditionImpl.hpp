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

#ifndef EPROSIMA_DDS_CORE_COND_TGUARDCONDITION_IMPL_HPP_
#define EPROSIMA_DDS_CORE_COND_TGUARDCONDITION_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/cond/GuardCondition.hpp>
//TODO: Fix when GuardConditionDelegate and ReportUtils are implemented
//#include <org/opensplice/core/cond/GuardConditionDelegate.hpp>
//#include <org/opensplice/core/ReportUtils.hpp>

// Implementation
namespace dds {
namespace core {
namespace cond {

template<typename DELEGATE>
TGuardCondition<DELEGATE>::TGuardCondition()
{
    //To implement
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template<typename DELEGATE>
template<typename FUN>
TGuardCondition<DELEGATE>::TGuardCondition(
        FUN& functor)
{
    //To implement
}

template<typename DELEGATE>
template<typename FUN>
TGuardCondition<DELEGATE>::TGuardCondition(
        const FUN& functor)
{
    //To implement
}
/** @endcond */

template<typename DELEGATE>
TGuardCondition<DELEGATE>::~TGuardCondition()
{
}

template<typename DELEGATE>
void TGuardCondition<DELEGATE>::trigger_value(
        bool value)
{
    //To implement
}

template<typename DELEGATE>
bool TGuardCondition<DELEGATE>::trigger_value()
{
    return TCondition<DELEGATE>::trigger_value();
}

//TODO: Fix when GuardConditionDelegate and ReportUtils are implemented
//template<typename DELEGATE>
//TCondition<DELEGATE>::TCondition(
//        const dds::core::cond::TGuardCondition<org::opensplice::core::cond::GuardConditionDelegate>& h)
//{
//    //To implement
//}

//TODO: Fix when GuardConditionDelegate and ReportUtils are implemented
//template<typename DELEGATE>
//TCondition<DELEGATE>&
//TCondition<DELEGATE>::operator =(
//        const dds::core::cond::TGuardCondition<org::opensplice::core::cond::GuardConditionDelegate>& rhs)
//{
//    //To implement
//}

} //namespace cond
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_COND_TGUARDCONDITION_IMPL_HPP_
