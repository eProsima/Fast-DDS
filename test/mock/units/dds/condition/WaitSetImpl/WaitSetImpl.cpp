// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of WaitSetImpl: replaces src/cpp/fastdds/core/condition/WaitSetImpl.cpp and forwards every
// method to the WaitSetImplMock companion of the instance.

#include <fastdds/core/condition/WaitSetImpl.hpp>

#include <mock/fastdds/core/condition/WaitSetImplMock.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

using Companion = mock::MockCompanion<WaitSetImpl>;

WaitSetImpl::WaitSetImpl()
{
    Companion::attach(this);
}

WaitSetImpl::~WaitSetImpl()
{
    Companion::detach(this);
}

ReturnCode_t WaitSetImpl::attach_condition(
        const Condition& condition)
{
    return Companion::get(this).attach_condition(condition);
}

ReturnCode_t WaitSetImpl::detach_condition(
        const Condition& condition)
{
    return Companion::get(this).detach_condition(condition);
}

ReturnCode_t WaitSetImpl::wait(
        ConditionSeq& active_conditions,
        const fastdds::dds::Duration_t& timeout)
{
    return Companion::get(this).wait(active_conditions, timeout);
}

ReturnCode_t WaitSetImpl::get_conditions(
        ConditionSeq& attached_conditions) const
{
    return Companion::get(this).get_conditions(attached_conditions);
}

void WaitSetImpl::wake_up()
{
    Companion::get(this).wake_up();
}

void WaitSetImpl::will_be_deleted (
        const Condition& condition)
{
    Companion::get(this).will_be_deleted(condition);
}

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
