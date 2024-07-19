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
 * @file GuidUtils.hpp
 */

#ifndef _STATISTICS_RTPS_GUIDUTILS_HPP_
#define _STATISTICS_RTPS_GUIDUTILS_HPP_

#include <fastdds/config.hpp>

#include <fastdds/rtps/common/EntityId_t.hpp>

#include <statistics/types/types.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {

/**
 * Checks whether an entity id corresponds to a builtin monitor service writer.
 * @param [in] entity_id The entity id to check.
 * @return true when the entity id corresponds to a builtin monitor service writer.
 */
inline bool is_monitor_service_builtin(
        const fastdds::rtps::EntityId_t& entity_id)
{
    bool ret = false;
#ifdef FASTDDS_STATISTICS
    ret = ENTITYID_MONITOR_SERVICE_WRITER == entity_id.to_uint32();
#endif // ifdef FASTDDS_STATISTICS
    static_cast<void>(entity_id);
    return ret;
}

/**
 * Checks whether an entity id corresponds to a builtin statistics writer.
 * @param [in] entity_id The entity id to check.
 * @return true when the entity id corresponds to a builtin statistics writer.
 */
inline bool is_statistics_builtin(
        const fastdds::rtps::EntityId_t& entity_id)
{
    return 0x60 == (0xE0 & entity_id.value[3]) || is_monitor_service_builtin(entity_id);
}

/**
 * Generate a builtin statistics writer entity id.
 * @param [in] kind The kind of the data being published by the builtin statistics writer.
 * @param [out] entity_id The corresponding entity id.
 */
inline void set_statistics_entity_id(
        uint32_t kind,
        fastdds::rtps::EntityId_t& entity_id)
{
    entity_id.value[3] = 0x62;
    entity_id.value[2] = kind & 0xFF;
    entity_id.value[1] = (kind >> 8) & 0xFF;
    entity_id.value[0] = (kind >> 16) & 0xFF;
}

} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _STATISTICS_RTPS_GUIDUTILS_HPP_
