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

#ifndef EPROSIMA_DDS_CORE_COND_TSTATUSCONDITION_IMPL_HPP_
#define EPROSIMA_DDS_CORE_COND_TSTATUSCONDITION_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/cond/StatusCondition.hpp>
//TODO: Fix when StatusConditionDelegate and ReportUtils are implemented
//#include <org/opensplice/core/cond/StatusConditionDelegate.hpp>
//#include <org/opensplice/core/ReportUtils.hpp>

namespace dds {
namespace core {
namespace cond {

template<typename DELEGATE>
TStatusCondition<DELEGATE>::TStatusCondition(
        const dds::core::Entity& e)
{
    //To implement
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template<typename DELEGATE>
template<typename FUN>
TStatusCondition<DELEGATE>::TStatusCondition(
        const dds::core::Entity& e,
        FUN& functor)
{
    //To implement
}

template<typename DELEGATE>
template<typename FUN>
TStatusCondition<DELEGATE>::TStatusCondition(
        const dds::core::Entity& e,
        const FUN& functor)
{
    //To implement
}
/** @endcond */

template<typename DELEGATE>
TStatusCondition<DELEGATE>::~TStatusCondition() { }

template<typename DELEGATE>
void TStatusCondition<DELEGATE>::enabled_statuses(
        const dds::core::status::StatusMask& status) const
{
    //To implement
}

template<typename DELEGATE>
const dds::core::status::StatusMask TStatusCondition<DELEGATE>::enabled_statuses() const
{
    //To implement
}

template<typename DELEGATE>
const dds::core::Entity& TStatusCondition<DELEGATE>::entity() const
{
    //To implement
}

//TODO: Fix when StatusConditionDelegate and ReportUtils are implemented
//template<typename DELEGATE>
//TCondition<DELEGATE>::TCondition(
//        const TStatusCondition<org::opensplice::core::cond::StatusConditionDelegate>& h)
//{
//    //To implement
//}

//TODO: Fix when StatusConditionDelegate and ReportUtils are implemented
//template<typename DELEGATE>
//TCondition<DELEGATE>& TCondition<DELEGATE>::operator =(
//        const dds::core::cond::TStatusCondition<org::opensplice::core::cond::StatusConditionDelegate>& rhs)
//{
//    //To implement
//}

} //namespace cond
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_COND_TSTATUSCONDITION_IMPL_HPP_
