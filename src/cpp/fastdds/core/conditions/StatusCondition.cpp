// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 *
 */

#include <fastdds/dds/core/conditions/StatusCondition.hpp>
#include <fastdds/dds/core/Entity.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

ReturnCode_t StatusCondition::set_enabled_statuses(
        StatusMask mask)
{
    enabled_statuses_ = mask;

    //Recalculate the trigger_value with the new mask
    StatusMask changed_statuses = entity_->get_status_changes();
    trigger_value_ = (changed_statuses & mask).any();

    return ReturnCode_t::RETCODE_OK;
}


} // namespace dds
} // namespace fastdds
} // namespace eprosima

