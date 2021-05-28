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
 * @file Condition.cpp
 */

#include <fastdds/dds/core/condition/Condition.hpp>

#include <fastdds/core/condition/ConditionNotifier.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

Condition::Condition()
    : notifier_ (new detail::ConditionNotifier())
{
}

Condition::~Condition()
{
    notifier_->will_be_deleted(*this);
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
