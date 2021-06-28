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
 * @file WaitSetImpl.hpp
 */

#ifndef _FASTDDS_CORE_CONDITION_WAITSETIMPL_HPP_
#define _FASTDDS_CORE_CONDITION_WAITSETIMPL_HPP_

#include <gmock/gmock.h>

#include <fastdds/dds/core/condition/Condition.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

struct WaitSetImpl
{
    /**
     * @brief Wake up this WaitSet implementation if it was waiting
     */
    MOCK_METHOD0(wake_up, void());

    /**
     * @brief Called from the destructor of a Condition to inform this WaitSet implementation that the condition
     * should be automatically detached.
     */
    MOCK_METHOD1(will_be_deleted, void(const Condition& condition));
};

}  // namespace detail
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // _FASTDDS_CORE_CONDITION_WAITSETIMPL_HPP_
