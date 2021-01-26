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
 * @file StatusCondition.cpp
 *
 */

#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {

using eprosima::fastrtps::types::ReturnCode_t;

ReturnCode_t StatusCondition::set_enabled_statuses(
        const StatusMask& mask)
{
    static_cast<void>(mask);
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

const StatusMask& StatusCondition::get_enabled_statuses() const
{
    logWarning(CONDITION, "get_enabled_statuses public member function not implemented");
    return status_mask;
}

Entity* StatusCondition::get_entity() const
{
    logWarning(CONDITION, "get_entity public member function not implemented");
    return nullptr;
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
