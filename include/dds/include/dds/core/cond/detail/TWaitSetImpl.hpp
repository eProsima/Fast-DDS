/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OSPL_DDS_CORE_COND_TWAITSET_HPP_
#define OSPL_DDS_CORE_COND_TWAITSET_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/cond/TWaitSet.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

// Implementation
namespace dds
{
namespace core
{
namespace cond
{
template <typename DELEGATE>
TWaitSet<DELEGATE>::TWaitSet()
{
    ISOCPP_REPORT_STACK_NC_BEGIN();
    this->set_ref(new DELEGATE);
    this->delegate()->init(this->impl_);
}

template <typename DELEGATE>
TWaitSet<DELEGATE>::~TWaitSet() { }

template <typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq TWaitSet<DELEGATE>::wait(const dds::core::Duration& timeout)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    ConditionSeq triggered;
    return this->wait(triggered, timeout);
}

template <typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq TWaitSet<DELEGATE>::wait()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    ConditionSeq triggered;
    return this->wait(triggered, dds::core::Duration::infinite());
}

template <typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::wait(ConditionSeq& triggered, const dds::core::Duration& timeout)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->wait(triggered, timeout);
}

template <typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::wait(ConditionSeq& triggered)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->wait(triggered, dds::core::Duration::infinite());
}

template <typename DELEGATE>
void TWaitSet<DELEGATE>::dispatch()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->dispatch(dds::core::Duration::infinite());
}

template <typename DELEGATE>
void TWaitSet<DELEGATE>::dispatch(const dds::core::Duration& timeout)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->dispatch(timeout);
}

template <typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::operator +=(const dds::core::cond::Condition& cond)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->attach_condition(cond);
    return *this;
}

template <typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::operator -=(const dds::core::cond::Condition& cond)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->detach_condition(cond);
    return *this;
}

template <typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::attach_condition(const dds::core::cond::Condition& cond)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->attach_condition(cond);
    return *this;
}

template <typename DELEGATE>
bool TWaitSet<DELEGATE>::detach_condition(const dds::core::cond::Condition& cond)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->detach_condition(cond.delegate().get());
}

template <typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq TWaitSet<DELEGATE>::conditions() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    ConditionSeq conds;
    return this->conditions(conds);
}

template <typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::conditions(ConditionSeq& conds) const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->conditions(conds);
}

}
}
}

// End of implementation

#endif /* OSPL_DDS_CORE_COND_TWAITSET_HPP_ */
