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

// Link-seam implementation of ConditionNotifier: replaces src/cpp/fastdds/core/condition/ConditionNotifier.cpp and
// forwards every method to the ConditionNotifierMock companion of the instance.

#include <fastdds/core/condition/ConditionNotifier.hpp>

#include <mock/fastdds/core/condition/ConditionNotifierMock.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

using Companion = mock::MockCompanion<ConditionNotifier>;

ConditionNotifier::ConditionNotifier()
{
    Companion::attach(this);
}

ConditionNotifier::~ConditionNotifier()
{
    Companion::detach(this);
}

void ConditionNotifier::attach_to (
        WaitSetImpl* wait_set)
{
    Companion::get(this).attach_to(wait_set);
}

void ConditionNotifier::detach_from (
        WaitSetImpl* wait_set)
{
    Companion::get(this).detach_from(wait_set);
}

void ConditionNotifier::notify ()
{
    Companion::get(this).notify();
}

void ConditionNotifier::will_be_deleted (
        const Condition& condition)
{
    Companion::get(this).will_be_deleted(condition);
}

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
