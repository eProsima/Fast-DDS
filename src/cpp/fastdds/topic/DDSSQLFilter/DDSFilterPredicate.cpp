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
 * @file DDSFilterPredicate.cpp
 */

#include "DDSFilterPredicate.hpp"

#include <cassert>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

DDSFilterPredicate::DDSFilterPredicate(
        OperationKind op,
        const std::shared_ptr<DDSFilterValue>& left,
        const std::shared_ptr<DDSFilterValue>& right)
    : op_(op)
    , left_(left)
    , right_(right)
{
    assert(left_);
    assert(right_);

    left_->add_parent(this);
    right_->add_parent(this);
}

void DDSFilterPredicate::value_has_changed(
        const DDSFilterValue& value)
{
    // TODO(Miguel C): Implement this
    static_cast<void>(value);
}

void DDSFilterPredicate::propagate_reset() noexcept
{
    left_->reset();
    right_->reset();
}

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
