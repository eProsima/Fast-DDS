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
#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>

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

static void from_builtin_to_proxy(
        const BuiltinTopicKeyValue& dds_key,
        EntityId_t& entity_id)
{
    entity_id.value[0] = static_cast<uint8_t>((dds_key[2] >> 24) & 0xFF);
    entity_id.value[1] = static_cast<uint8_t>((dds_key[2] >> 16) & 0xFF);
    entity_id.value[2] = static_cast<uint8_t>((dds_key[2] >> 8) & 0xFF);
    entity_id.value[3] = static_cast<uint8_t>(dds_key[2] & 0xFF);
}

static void from_builtin_to_proxy(
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
    from_proxy_to_builtin(proxy_data.guid().entityId, builtin_data.key.value);
    from_proxy_to_builtin(proxy_data.guid().guidPrefix, builtin_data.participant_key.value);

    builtin_data.topic_name = proxy_data.topicName();
    builtin_data.type_name = proxy_data.typeName();
    builtin_data.durability = proxy_data.m_qos.m_durability;
    builtin_data.deadline = proxy_data.m_qos.m_deadline;
    builtin_data.latency_budget = proxy_data.m_qos.m_latencyBudget;
    builtin_data.lifespan = proxy_data.m_qos.m_lifespan;
    builtin_data.liveliness = proxy_data.m_qos.m_liveliness;
    builtin_data.reliability = proxy_data.m_qos.m_reliability;
    builtin_data.ownership = proxy_data.m_qos.m_ownership;
    builtin_data.destination_order = proxy_data.m_qos.m_destinationOrder;
    builtin_data.user_data = proxy_data.m_qos.m_userData;
    builtin_data.time_based_filter = proxy_data.m_qos.m_timeBasedFilter;

    builtin_data.presentation = proxy_data.m_qos.m_presentation;
    builtin_data.partition = proxy_data.m_qos.m_partition;
    builtin_data.topic_data = proxy_data.m_qos.m_topicData;
    builtin_data.group_data = proxy_data.m_qos.m_groupData;

    if (proxy_data.has_type_information())
    {
        builtin_data.type_information = proxy_data.type_information();
    }
    builtin_data.representation = proxy_data.m_qos.representation;
    builtin_data.type_consistency = proxy_data.m_qos.type_consistency;

    builtin_data.content_filter = proxy_data.content_filter();
    builtin_data.disable_positive_acks = proxy_data.m_qos.m_disablePositiveACKs;
    builtin_data.data_sharing = proxy_data.m_qos.data_sharing;
    builtin_data.guid = proxy_data.guid();
    builtin_data.participant_guid = iHandle2GUID(proxy_data.RTPSParticipantKey());
    builtin_data.remote_locators = proxy_data.remote_locators();
    builtin_data.loopback_transformation = proxy_data.networkConfiguration();
    builtin_data.expects_inline_qos = proxy_data.m_expectsInlineQos;
}

void from_proxy_to_builtin(
        const WriterProxyData& proxy_data,
        PublicationBuiltinTopicData& builtin_data)
{
    from_proxy_to_builtin(proxy_data.guid().entityId, builtin_data.key.value);
    from_proxy_to_builtin(proxy_data.guid().guidPrefix, builtin_data.participant_key.value);

    builtin_data.topic_name = proxy_data.topicName();
    builtin_data.type_name = proxy_data.typeName();
    builtin_data.durability = proxy_data.m_qos.m_durability;
    builtin_data.durability_service = proxy_data.m_qos.m_durabilityService;
    builtin_data.deadline = proxy_data.m_qos.m_deadline;
    builtin_data.latency_budget = proxy_data.m_qos.m_latencyBudget;
    builtin_data.liveliness = proxy_data.m_qos.m_liveliness;
    builtin_data.reliability = proxy_data.m_qos.m_reliability;
    builtin_data.lifespan = proxy_data.m_qos.m_lifespan;
    builtin_data.user_data = proxy_data.m_qos.m_userData;
    builtin_data.ownership = proxy_data.m_qos.m_ownership;
    builtin_data.ownership_strength = proxy_data.m_qos.m_ownershipStrength;
    builtin_data.destination_order = proxy_data.m_qos.m_destinationOrder;

    builtin_data.presentation = proxy_data.m_qos.m_presentation;
    builtin_data.partition = proxy_data.m_qos.m_partition;
    builtin_data.topic_data = proxy_data.m_qos.m_topicData;
    builtin_data.group_data = proxy_data.m_qos.m_groupData;

    if (proxy_data.has_type_information())
    {
        builtin_data.type_information = proxy_data.type_information();
    }
    builtin_data.representation = proxy_data.m_qos.representation;

    builtin_data.disable_positive_acks = proxy_data.m_qos.m_disablePositiveACKs;
    builtin_data.data_sharing = proxy_data.m_qos.data_sharing;
    builtin_data.guid = proxy_data.guid();
    builtin_data.persistence_guid = proxy_data.persistence_guid();
    builtin_data.participant_guid = iHandle2GUID(proxy_data.RTPSParticipantKey());
    builtin_data.remote_locators = proxy_data.remote_locators();
    builtin_data.max_serialized_size = proxy_data.typeMaxSerialized();
    builtin_data.loopback_transformation = proxy_data.networkConfiguration();
}

void from_builtin_to_proxy(
        const PublicationBuiltinTopicData& builtin_data,
        WriterProxyData& proxy_data)
{
    from_builtin_to_proxy(builtin_data.participant_key.value, proxy_data.guid().guidPrefix);
    from_builtin_to_proxy(builtin_data.key.value, proxy_data.guid().entityId);

    proxy_data.topicName(builtin_data.topic_name);
    proxy_data.typeName(builtin_data.type_name);
    proxy_data.m_qos.m_durability = builtin_data.durability;
    proxy_data.m_qos.m_durabilityService = builtin_data.durability_service;
    proxy_data.m_qos.m_deadline = builtin_data.deadline;
    proxy_data.m_qos.m_latencyBudget = builtin_data.latency_budget;
    proxy_data.m_qos.m_liveliness = builtin_data.liveliness;
    proxy_data.m_qos.m_reliability = builtin_data.reliability;
    proxy_data.m_qos.m_lifespan = builtin_data.lifespan;
    proxy_data.m_qos.m_userData = builtin_data.user_data;
    proxy_data.m_qos.m_ownership = builtin_data.ownership;
    proxy_data.m_qos.m_ownershipStrength = builtin_data.ownership_strength;
    proxy_data.m_qos.m_destinationOrder = builtin_data.destination_order;

    proxy_data.m_qos.m_presentation = builtin_data.presentation;
    proxy_data.m_qos.m_partition = builtin_data.partition;
    proxy_data.m_qos.m_topicData = builtin_data.topic_data;
    proxy_data.m_qos.m_groupData = builtin_data.group_data;

    proxy_data.type_information(builtin_data.type_information);
    proxy_data.m_qos.representation = builtin_data.representation;

    proxy_data.m_qos.m_disablePositiveACKs = builtin_data.disable_positive_acks;
    proxy_data.m_qos.data_sharing = builtin_data.data_sharing;
    proxy_data.guid(builtin_data.guid);
    proxy_data.persistence_guid(builtin_data.persistence_guid);
    proxy_data.RTPSParticipantKey(builtin_data.participant_guid);
    proxy_data.set_locators(builtin_data.remote_locators);
    proxy_data.typeMaxSerialized(builtin_data.max_serialized_size);
    proxy_data.networkConfiguration(builtin_data.loopback_transformation);
}

void from_builtin_to_proxy(
        const SubscriptionBuiltinTopicData& builtin_data,
        ReaderProxyData& proxy_data)
{
    from_builtin_to_proxy(builtin_data.participant_key.value, proxy_data.guid().guidPrefix);
    from_builtin_to_proxy(builtin_data.key.value, proxy_data.guid().entityId);

    proxy_data.topicName(builtin_data.topic_name);
    proxy_data.typeName(builtin_data.type_name);
    proxy_data.m_qos.m_durability = builtin_data.durability;
    proxy_data.m_qos.m_deadline = builtin_data.deadline;
    proxy_data.m_qos.m_latencyBudget = builtin_data.latency_budget;
    proxy_data.m_qos.m_lifespan = builtin_data.lifespan;
    proxy_data.m_qos.m_liveliness = builtin_data.liveliness;
    proxy_data.m_qos.m_reliability = builtin_data.reliability;
    proxy_data.m_qos.m_ownership = builtin_data.ownership;
    proxy_data.m_qos.m_destinationOrder = builtin_data.destination_order;
    proxy_data.m_qos.m_userData = builtin_data.user_data;
    proxy_data.m_qos.m_timeBasedFilter = builtin_data.time_based_filter;

    proxy_data.m_qos.m_presentation = builtin_data.presentation;
    proxy_data.m_qos.m_partition = builtin_data.partition;
    proxy_data.m_qos.m_topicData = builtin_data.topic_data;
    proxy_data.m_qos.m_groupData = builtin_data.group_data;

    proxy_data.type_information(builtin_data.type_information);
    proxy_data.m_qos.representation = builtin_data.representation;
    proxy_data.m_qos.type_consistency = builtin_data.type_consistency;

    proxy_data.content_filter(builtin_data.content_filter);
    proxy_data.m_qos.m_disablePositiveACKs = builtin_data.disable_positive_acks;
    proxy_data.m_qos.data_sharing = builtin_data.data_sharing;
    proxy_data.guid(builtin_data.guid);
    proxy_data.RTPSParticipantKey(builtin_data.participant_guid);
    proxy_data.set_locators(builtin_data.remote_locators);
    proxy_data.networkConfiguration(builtin_data.loopback_transformation);
    proxy_data.m_expectsInlineQos = builtin_data.expects_inline_qos;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
