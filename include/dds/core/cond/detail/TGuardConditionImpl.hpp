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
    //    ISOCPP_REPORT_STACK_NC_BEGIN();
    //    this->set_ref(new DELEGATE);
    //    this->delegate()->init(this->impl_);
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
    //    ISOCPP_REPORT_STACK_NC_BEGIN();
    //    this->set_ref(new DELEGATE);
    //    this->delegate()->init(this->impl_);
    //    this->delegate()->set_handler(functor);
}

template<typename DELEGATE>
template<typename FUN>
TGuardCondition<DELEGATE>::TGuardCondition(
        const FUN& functor)
{
    //To implement
    //    ISOCPP_REPORT_STACK_NC_BEGIN();
    //    this->set_ref(new DELEGATE);
    //    this->delegate()->init(this->impl_);
    //    this->delegate()->set_handler(functor);
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
    //    ISOCPP_REPORT_STACK_NC_BEGIN();
    //    this->delegate()->trigger_value(value);
}

template<typename DELEGATE>
bool TGuardCondition<DELEGATE>::trigger_value()
{
    //To implement
    //    return TCondition<DELEGATE>::trigger_value();
}

template<typename DELEGATE>
TCondition<DELEGATE>::TCondition(
        /*const dds::core::cond::TGuardCondition<org::opensplice::core::cond::GuardConditionDelegate>& h*/)
{
    //To implement
    //    ISOCPP_REPORT_STACK_NC_BEGIN();
    //    if (h.is_nil()) {
    //        /* We got a null object and are not really able to do a typecheck here. */
    //        /* So, just set a null object. */
    //        *this = dds::core::null;
    //    } else {
    //        this->::dds::core::Reference<DELEGATE>::impl_ = OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<DELEGATE_T>(h.delegate());
    //        if (h.delegate() != this->::dds::core::Reference<DELEGATE>::impl_) {
    //            throw dds::core::IllegalOperationError(std::string("Attempted invalid cast: ") + typeid(h).name() + " to " + typeid(*this).name());
    //        }
    //    }
}

template<typename DELEGATE>
TCondition<DELEGATE>& TCondition<DELEGATE>::operator =(
        /*const dds::core::cond::TGuardCondition<org::opensplice::core::cond::GuardConditionDelegate>& rhs*/)
{
    //To implement
    //    ISOCPP_REPORT_STACK_NC_BEGIN();
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

#endif //EPROSIMA_DDS_CORE_COND_TGUARDCONDITION_IMPL_HPP_
