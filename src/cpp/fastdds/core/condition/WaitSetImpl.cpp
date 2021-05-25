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
 * @file WaitSetImpl.cpp
 */

#include "WaitSetImpl.hpp"

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

ReturnCode_t WaitSetImpl::attach_condition(
        const Condition& condition)
{
    static_cast<void>(condition);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t WaitSetImpl::detach_condition(
        const Condition& condition)
{
    static_cast<void>(condition);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t WaitSetImpl::wait(
        ConditionSeq& active_conditions,
        const fastrtps::Duration_t& timeout)
{
    static_cast<void>(active_conditions);
    static_cast<void>(timeout);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t WaitSetImpl::get_conditions(
        ConditionSeq& attached_conditions) const
{
    static_cast<void>(attached_conditions);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

void WaitSetImpl::wake_up()
{
}

void WaitSetImpl::will_be_deleted (
        const Condition& condition)
{
    static_cast<void>(condition);
}

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
