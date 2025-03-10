// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BuiltinTopicKeyConversions.hpp
 */

#ifndef UTILS__BUILTIN_TOPIC_KEY_CONVERSIONS_HPP_
#define UTILS__BUILTIN_TOPIC_KEY_CONVERSIONS_HPP_

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

typedef uint32_t BuiltinTopicKeyValue[3];

inline void from_entity_id_to_topic_key(
        const EntityId_t& entity_id,
        BuiltinTopicKeyValue& builtin_key_value)
{
    builtin_key_value[0] = 0;
    builtin_key_value[1] = 0;
    builtin_key_value[2] = static_cast<uint32_t>(entity_id.value[0]) << 24
            | static_cast<uint32_t>(entity_id.value[1]) << 16
            | static_cast<uint32_t>(entity_id.value[2]) << 8
            | static_cast<uint32_t>(entity_id.value[3]);
}

inline void from_guid_prefix_to_topic_key(
        const GuidPrefix_t& guid_prefix,
        BuiltinTopicKeyValue& dds_key)
{
    dds_key[0] = static_cast<uint32_t>(guid_prefix.value[0]) << 24
            | static_cast<uint32_t>(guid_prefix.value[1]) << 16
            | static_cast<uint32_t>(guid_prefix.value[2]) << 8
            | static_cast<uint32_t>(guid_prefix.value[3]);
    dds_key[1] = static_cast<uint32_t>(guid_prefix.value[4]) << 24
            | static_cast<uint32_t>(guid_prefix.value[5]) << 16
            | static_cast<uint32_t>(guid_prefix.value[6]) << 8
            | static_cast<uint32_t>(guid_prefix.value[7]);
    dds_key[2] = static_cast<uint32_t>(guid_prefix.value[8]) << 24
            | static_cast<uint32_t>(guid_prefix.value[9]) << 16
            | static_cast<uint32_t>(guid_prefix.value[10]) << 8
            | static_cast<uint32_t>(guid_prefix.value[11]);
}

inline void from_topic_key_to_entity_id(
        const BuiltinTopicKeyValue& dds_key,
        EntityId_t& entity_id)
{
    entity_id.value[0] = static_cast<uint8_t>((dds_key[2] >> 24) & 0xFF);
    entity_id.value[1] = static_cast<uint8_t>((dds_key[2] >> 16) & 0xFF);
    entity_id.value[2] = static_cast<uint8_t>((dds_key[2] >> 8) & 0xFF);
    entity_id.value[3] = static_cast<uint8_t>(dds_key[2] & 0xFF);
}

inline void from_topic_key_to_guid_prefix(
        const BuiltinTopicKeyValue& dds_key,
        GuidPrefix_t& guid_prefix)
{
    guid_prefix.value[0] = static_cast<uint8_t>((dds_key[0] >> 24) & 0xFF);
    guid_prefix.value[1] = static_cast<uint8_t>((dds_key[0] >> 16) & 0xFF);
    guid_prefix.value[2] = static_cast<uint8_t>((dds_key[0] >> 8) & 0xFF);
    guid_prefix.value[3] = static_cast<uint8_t>(dds_key[0] & 0xFF);

    guid_prefix.value[4] = static_cast<uint8_t>((dds_key[1] >> 24) & 0xFF);
    guid_prefix.value[5] = static_cast<uint8_t>((dds_key[1] >> 16) & 0xFF);
    guid_prefix.value[6] = static_cast<uint8_t>((dds_key[1] >> 8) & 0xFF);
    guid_prefix.value[7] = static_cast<uint8_t>(dds_key[1] & 0xFF);

    guid_prefix.value[8] = static_cast<uint8_t>((dds_key[2] >> 24) & 0xFF);
    guid_prefix.value[9] = static_cast<uint8_t>((dds_key[2] >> 16) & 0xFF);
    guid_prefix.value[10] = static_cast<uint8_t>((dds_key[2] >> 8) & 0xFF);
    guid_prefix.value[11] = static_cast<uint8_t>(dds_key[2] & 0xFF);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // UTILS__BUILTIN_TOPIC_KEY_CONVERSIONS_HPP_
