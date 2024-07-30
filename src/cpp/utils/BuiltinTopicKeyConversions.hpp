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

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

typedef uint32_t BuiltinTopicKeyValue[3];

void from_proxy_to_builtin(
        const EntityId_t& entity_id,
        BuiltinTopicKeyValue& builtin_key_value);

void from_proxy_to_builtin(
        const GuidPrefix_t& guid_prefix,
        BuiltinTopicKeyValue& dds_key);

void from_builtin_to_proxy(
        const BuiltinTopicKeyValue& dds_key,
        EntityId_t& entity_id);

void from_builtin_to_proxy(
        const BuiltinTopicKeyValue& dds_key,
        GuidPrefix_t& guid_prefix);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
