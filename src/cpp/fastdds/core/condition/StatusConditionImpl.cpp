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

#include <mutex>

#include <fastdds/core/condition/ConditionNotifier.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

StatusConditionImpl::StatusConditionImpl(
        ConditionNotifier* notifier)
    : mask_(StatusMask::all())
    , status_(StatusMask::none())
    , notifier_(notifier)
{
}

StatusConditionImpl::~StatusConditionImpl()
{
}

bool StatusConditionImpl::get_trigger_value() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return (mask_ & status_).any();
}

ReturnCode_t StatusConditionImpl::set_enabled_statuses(
        const StatusMask& mask)
{
    bool notify = false;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        bool old_trigger = (mask_ & status_).any();
        mask_ = mask;
        bool new_trigger = (mask_ & status_).any();
        notify = !old_trigger && new_trigger;
    }

    if (notify)
    {
        notifier_->notify();
    }
    return RETCODE_OK;
}

const StatusMask& StatusConditionImpl::get_enabled_statuses() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return mask_;
}

void StatusConditionImpl::set_status(
        const StatusMask& status,
        bool trigger_value)
{
    if (trigger_value)
    {
        bool notify = false;
        {
            std::lock_guard<std::mutex> guard(mutex_);
            bool old_trigger = (mask_ & status_).any();
            status_ |= status;
            bool new_trigger = (mask_ & status_).any();
            notify = !old_trigger && new_trigger;
        }

        if (notify)
        {
            notifier_->notify();
        }
    }
    else
    {
        std::lock_guard<std::mutex> guard(mutex_);
        status_ &= ~status;
    }
}

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
