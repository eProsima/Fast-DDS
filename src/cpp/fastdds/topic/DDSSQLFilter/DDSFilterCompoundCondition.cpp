// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DDSFilterCompoundCondition.cpp
 */

#include "DDSFilterCompoundCondition.hpp"

#include <cassert>
#include <memory>

#include "DDSFilterCondition.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

DDSFilterCompoundCondition::DDSFilterCompoundCondition(
        OperationKind op,
        std::unique_ptr<DDSFilterCondition>&& left,
        std::unique_ptr<DDSFilterCondition>&& right)
    : op_(op)
    , left_(std::move(left))
    , right_(std::move(right))
{
    assert(left_);
    assert(right_ || OperationKind::NOT == op_);

    left_->set_parent(this);
    if (right_)
    {
        right_->set_parent(this);
    }
}

void DDSFilterCompoundCondition::propagate_reset() noexcept
{
    // TODO(Miguel C): Implement this
}

void DDSFilterCompoundCondition::child_has_changed(
        const DDSFilterCondition& child) noexcept
{
    // TODO(Miguel C): Implement this
    static_cast<void>(child);
}

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
