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
 * @file StatusConditionImpl.cpp
 */

#include "StatusConditionImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

StatusConditionImpl::StatusConditionImpl(
        ConditionNotifier* notifier)
{
    static_cast<void>(notifier);
}

StatusConditionImpl::~StatusConditionImpl()
{
}

bool StatusConditionImpl::get_trigger_value() const
{
    return false;
}

ReturnCode_t StatusConditionImpl::set_enabled_statuses(
        const StatusMask& mask)
{
    static_cast<void>(mask);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

const StatusMask& StatusConditionImpl::get_enabled_statuses() const
{
    static const StatusMask none = StatusMask::none();
    return none;
}

void StatusConditionImpl::set_status(
        const StatusMask& status,
        bool trigger_value)
{
    static_cast<void>(status);
    static_cast<void>(trigger_value);
}

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
