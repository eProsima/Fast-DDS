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
 * @file WaitSet.cpp
 *
 */

#include <fastdds/dds/core/condition/WaitSet.hpp>

#include <fastdds/core/condition/WaitSetImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

WaitSet::WaitSet()
    : impl_(new detail::WaitSetImpl())
{
}

WaitSet::~WaitSet()
{
}

ReturnCode_t WaitSet::attach_condition(
        const Condition& cond)
{
    return impl_->attach_condition(cond);
}

ReturnCode_t WaitSet::detach_condition(
        const Condition& cond)
{
    return impl_->detach_condition(cond);
}

ReturnCode_t WaitSet::wait(
        ConditionSeq& active_conditions,
        const fastdds::dds::Duration_t timeout) const
{
    return impl_->wait(active_conditions, timeout);
}

ReturnCode_t WaitSet::get_conditions(
        ConditionSeq& attached_conditions) const
{
    return impl_->get_conditions(attached_conditions);
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
