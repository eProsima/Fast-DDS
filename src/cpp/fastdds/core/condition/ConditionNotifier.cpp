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
 * @file ConditionNotifier.cpp
 */

#include "ConditionNotifier.hpp"
#include <fastdds/dds/core/condition/Condition.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

void ConditionNotifier::attach_to (
        WaitSetImpl* wait_set)
{
    static_cast<void>(wait_set);
}

void ConditionNotifier::detach_from (
        WaitSetImpl* wait_set)
{
    static_cast<void>(wait_set);
}

void ConditionNotifier::notify ()
{
}

void ConditionNotifier::will_be_deleted (
        const Condition& condition)
{
    static_cast<void>(condition);
}

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
