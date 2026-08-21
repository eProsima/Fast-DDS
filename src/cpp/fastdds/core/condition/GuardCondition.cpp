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
 * @file GuardCondition.cpp
 */

#include <fastdds/dds/core/condition/GuardCondition.hpp>

#include <fastdds/core/condition/ConditionNotifier.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

GuardCondition::GuardCondition()
    : trigger_value_(false)
{
}

GuardCondition::~GuardCondition()
{
    // Detach here, while this is still a fully constructed GuardCondition.
    // Condition::~Condition() also detaches, but it runs after the vtable has
    // been rewritten, so a concurrent WaitSetImpl::wait() may call
    // get_trigger_value() on a partially destroyed object.
    notifier_->will_be_deleted(*this);
}

bool GuardCondition::get_trigger_value() const
{
    return trigger_value_.load();
}

ReturnCode_t GuardCondition::set_trigger_value(
        bool value)
{
    bool old_value = trigger_value_.exchange(value);
    if (!old_value && value)
    {
        notifier_->notify();
    }

    return RETCODE_OK;
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
