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
 * @file WaitSet.cpp
 *
 */

#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {

fastrtps::types::ReturnCode_t WaitSet::attach_condition(const Condition& cond)
{
    static_cast<void>(cond);
    return fastrtps::types::ReturnCode_t::RETCODE_UNSUPPORTED;
}

fastrtps::types::ReturnCode_t WaitSet::detach_condition(const Condition& cond)
{
    static_cast<void>(cond);
    return fastrtps::types::ReturnCode_t::RETCODE_UNSUPPORTED;
}

fastrtps::types::ReturnCode_t WaitSet::wait(ConditionSeq& active_conditions, const fastrtps::Duration_t timeout)
{
    static_cast<void>(active_conditions);
    static_cast<void>(timeout);
    return fastrtps::types::ReturnCode_t::RETCODE_UNSUPPORTED;
}

fastrtps::types::ReturnCode_t WaitSet::get_conditions(ConditionSeq& attached_conditions)
{
    static_cast<void>(attached_conditions);
    return fastrtps::types::ReturnCode_t::RETCODE_UNSUPPORTED;
}


}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
