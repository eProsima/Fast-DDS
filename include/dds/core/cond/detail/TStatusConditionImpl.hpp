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
//    ISOCPP_REPORT_STACK_DDS_BEGIN(e);
//    dds::core::Reference<DELEGATE>::impl_=
//            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::core::cond::StatusConditionDelegate>(
//                        e.delegate()->get_statusCondition());
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
//    ISOCPP_REPORT_STACK_DDS_BEGIN(e);
//    dds::core::Reference<DELEGATE>::impl_=
//            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::core::cond::StatusConditionDelegate>(
//                          e.delegate()->get_statusCondition());
//    this->delegate()->set_handler(functor);
}

template<typename DELEGATE>
template<typename FUN>
TStatusCondition<DELEGATE>::TStatusCondition(
        const dds::core::Entity& e,
        const FUN& functor)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(e);
//    dds::core::Reference<DELEGATE>::impl_=
//            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::core::cond::StatusConditionDelegate>(
//                          e.delegate()->get_statusCondition());
//    this->delegate()->set_handler(functor);
}
/** @endcond */

template<typename DELEGATE>
TStatusCondition<DELEGATE>::~TStatusCondition()
{
}

template<typename DELEGATE>
void TStatusCondition<DELEGATE>::enabled_statuses(
        const dds::core::status::StatusMask& status) const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->enabled_statuses(status);
}

template<typename DELEGATE>
const dds::core::status::StatusMask TStatusCondition<DELEGATE>::enabled_statuses() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->enabled_statuses();
}

template<typename DELEGATE>
const dds::core::Entity& TStatusCondition<DELEGATE>::entity() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->entity();
}


template<typename DELEGATE>
TCondition<DELEGATE>::TCondition(
        /*const TStatusCondition<org::opensplice::core::cond::StatusConditionDelegate>& h*/)
{
    //To implement
//    if (h.is_nil()) {
//        /* We got a null object and are not really able to do a typecheck here. */
//        /* So, just set a null object. */
//        *this = dds::core::null;
//    } else {
//        ISOCPP_REPORT_STACK_DDS_BEGIN(h);

//        this->::dds::core::Reference<DELEGATE>::impl_ = OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DELEGATE_T>(h.delegate());
//        if (h.delegate() != this->::dds::core::Reference<DELEGATE>::impl_) {
//            throw dds::core::IllegalOperationError(std::string("Attempted invalid cast: ") + typeid(h).name() + " to " + typeid(*this).name());
//        }
//    }
}

template<typename DELEGATE>
TCondition<DELEGATE>& TCondition<DELEGATE>::operator =(
        /*const dds::core::cond::TStatusCondition<org::opensplice::core::cond::StatusConditionDelegate>& rhs*/)
{
    //To implement
//    if (this != (TCondition*)&rhs) {
//        if (rhs.is_nil()) {
//            /* We got a null object and are not really able to do a typecheck here. */
//            /* So, just set a null object. */
//            *this = dds::core::null;
//        } else {
//            TCondition other(rhs);
//            /* Dont have to copy when the delegate is the same. */
//            if (other.delegate() != this->::dds::core::Reference<DELEGATE>::impl_) {
//                *this = other;
//            }
//        }
//    }
//    return *this;
}

} //namespace cond
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_COND_TSTATUSCONDITION_IMPL_HPP_
