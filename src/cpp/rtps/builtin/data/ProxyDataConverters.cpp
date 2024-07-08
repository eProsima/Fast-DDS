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
 * @file ProxyDataConverters.cpp
 */

#include <rtps/builtin/data/ProxyDataConverters.hpp>

#include <cstdint>

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/builtin/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.hpp>
#include <fastdds/rtps/builtin/data/ReaderProxyData.hpp>
#include <fastdds/rtps/builtin/data/WriterProxyData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

typedef uint32_t BuiltinTopicKeyValue[3];

static void from_proxy_to_builtin(
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

static void from_proxy_to_builtin(
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

void from_proxy_to_builtin(
        const ParticipantProxyData& proxy_data,
        ParticipantBuiltinTopicData& builtin_data)
{
    static_cast<void>(proxy_data);
    static_cast<void>(builtin_data);
}

void from_proxy_to_builtin(
        const ReaderProxyData& proxy_data,
        SubscriptionBuiltinTopicData& builtin_data)
{
    static_cast<void>(proxy_data);
    static_cast<void>(builtin_data);
}

void from_proxy_to_builtin(
        const WriterProxyData& proxy_data,
        PublicationBuiltinTopicData& builtin_data)
{
    static_cast<void>(proxy_data);
    static_cast<void>(builtin_data);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
