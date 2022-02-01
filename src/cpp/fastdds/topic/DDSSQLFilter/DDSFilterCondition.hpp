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
 * @file DDSFilterCondition.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERCONDITION_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERCONDITION_HPP_

#include "DDSFilterConditionState.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

struct DDSFilterCondition
{
    virtual ~DDSFilterCondition() = default;

    DDSFilterConditionState get_state() const noexcept;

    virtual void reset() noexcept = 0;

protected:

    void set_state(
            DDSFilterConditionState state) noexcept;

    virtual void child_has_changed(
            const DDSFilterCondition& child) noexcept = 0;

};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERCONDITION_HPP_
