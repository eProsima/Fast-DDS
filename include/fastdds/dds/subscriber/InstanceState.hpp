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
 * @file InstanceState.hpp
 */

#ifndef _FASTDDS_DDS_SUBSCRIBER_INSTANCESTATE_HPP_
#define _FASTDDS_DDS_SUBSCRIBER_INSTANCESTATE_HPP_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

enum DDSInstanceStateKind : uint16_t
{
    ALIVE_INSTANCE_STATE                = 0x0001 << 0,
    NOT_ALIVE_DISPOSED_INSTANCE_STATE   = 0x0001 << 1,
    NOT_ALIVE_NO_WRITERS_INSTANCE_STATE = 0x0001 << 2
};

using DDSInstanceStateMask = uint16_t;

constexpr DDSInstanceStateMask NOT_ALIVE_INSTANCE_STATE =
        NOT_ALIVE_DISPOSED_INSTANCE_STATE | NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
constexpr DDSInstanceStateMask ANY_INSTANCE_STATE = ALIVE_INSTANCE_STATE | NOT_ALIVE_INSTANCE_STATE;

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_DDS_SUBSCRIBER_INSTANCESTATE_HPP_
