// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PublicationBuiltinTopicData.cpp
 */

#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>

#include <fastdds/dds/publisher/qos/WriterQos.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

void PublicationBuiltinTopicData::set_qos(
        const PublicationBuiltinTopicData& qos,
        bool first_time)
{
    if (first_time)
    {
        durability = qos.durability;
        durability.hasChanged = true;
    }
    if (first_time || deadline.period != qos.deadline.period)
    {
        deadline = qos.deadline;
        deadline.hasChanged = true;
    }
    if (latency_budget.duration != qos.latency_budget.duration)
    {
        latency_budget = qos.latency_budget;
        latency_budget.hasChanged = true;
    }
    if (first_time)
    {
        liveliness = qos.liveliness;
        liveliness.hasChanged = true;
    }
    if (first_time)
    {
        reliability = qos.reliability;
        reliability.hasChanged = true;
    }
    if (first_time)
    {
        ownership = qos.ownership;
        ownership.hasChanged = true;
    }
    if (destination_order.kind != qos.destination_order.kind)
    {
        destination_order = qos.destination_order;
        destination_order.hasChanged = true;
    }
    if (first_time || user_data.data_vec() != qos.user_data.data_vec())
    {
        user_data = qos.user_data;
        user_data.hasChanged = true;
    }
    if (first_time || presentation.access_scope != qos.presentation.access_scope ||
            presentation.coherent_access != qos.presentation.coherent_access ||
            presentation.ordered_access != qos.presentation.ordered_access)
    {
        presentation = qos.presentation;
        presentation.hasChanged = true;
    }
    if (first_time || qos.partition.names() != partition.names())
    {
        partition = qos.partition;
        partition.hasChanged = true;
    }

    if (first_time || topic_data.getValue() != qos.topic_data.getValue())
    {
        topic_data = qos.topic_data;
        topic_data.hasChanged = true;
    }
    if (first_time || group_data.getValue() != qos.group_data.getValue())
    {
        group_data = qos.group_data;
        group_data.hasChanged = true;
    }
    if (first_time || durability_service.history_kind != qos.durability_service.history_kind ||
            durability_service.history_depth != qos.durability_service.history_depth ||
            durability_service.max_instances != qos.durability_service.max_instances ||
            durability_service.max_samples != qos.durability_service.max_samples ||
            durability_service.max_samples_per_instance != qos.durability_service.max_samples_per_instance ||
            durability_service.service_cleanup_delay != qos.durability_service.service_cleanup_delay
            )
    {
        durability_service = qos.durability_service;
        durability_service.hasChanged = true;
    }
    if (lifespan.duration != qos.lifespan.duration)
    {
        lifespan = qos.lifespan;
        lifespan.hasChanged = true;
    }
    if (qos.ownership_strength.value != ownership_strength.value)
    {
        ownership_strength = qos.ownership_strength;
        ownership_strength.hasChanged = true;
    }
    if (first_time)
    {
        disable_positive_acks = qos.disable_positive_acks;
        disable_positive_acks.hasChanged = true;
    }
    // Writers only manages the first element in the list of data representations.
    if (qos.representation.m_value.size() != representation.m_value.size() ||
            (qos.representation.m_value.size() > 0 && representation.m_value.size() > 0 &&
            *qos.representation.m_value.begin() != *representation.m_value.begin()))
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }
    if (first_time && !(data_sharing == qos.data_sharing))
    {
        data_sharing = qos.data_sharing;
        data_sharing.hasChanged = true;
    }
    if (first_time)
    {
        publish_mode = qos.publish_mode;
        publish_mode.hasChanged = true;
    }
}

void PublicationBuiltinTopicData::set_qos(
        const dds::WriterQos& qos,
        bool first_time)
{
    if (first_time)
    {
        durability = qos.m_durability;
        durability.hasChanged = true;
    }
    if (first_time || deadline.period != qos.m_deadline.period)
    {
        deadline = qos.m_deadline;
        deadline.hasChanged = true;
    }
    if (latency_budget.duration != qos.m_latencyBudget.duration)
    {
        latency_budget = qos.m_latencyBudget;
        latency_budget.hasChanged = true;
    }
    if (first_time)
    {
        liveliness = qos.m_liveliness;
        liveliness.hasChanged = true;
    }
    if (first_time)
    {
        reliability = qos.m_reliability;
        reliability.hasChanged = true;
    }
    if (first_time)
    {
        ownership = qos.m_ownership;
        ownership.hasChanged = true;
    }
    if (destination_order.kind != qos.m_destinationOrder.kind)
    {
        destination_order = qos.m_destinationOrder;
        destination_order.hasChanged = true;
    }
    if (first_time || user_data.data_vec() != qos.m_userData.data_vec())
    {
        user_data = qos.m_userData;
        user_data.hasChanged = true;
    }
    if (first_time || presentation.access_scope != qos.m_presentation.access_scope ||
            presentation.coherent_access != qos.m_presentation.coherent_access ||
            presentation.ordered_access != qos.m_presentation.ordered_access)
    {
        presentation = qos.m_presentation;
        presentation.hasChanged = true;
    }
    if (first_time || qos.m_partition.names() != partition.names())
    {
        partition = qos.m_partition;
        partition.hasChanged = true;
    }

    if (first_time || topic_data.getValue() != qos.m_topicData.getValue())
    {
        topic_data = qos.m_topicData;
        topic_data.hasChanged = true;
    }
    if (first_time || group_data.getValue() != qos.m_groupData.getValue())
    {
        group_data = qos.m_groupData;
        group_data.hasChanged = true;
    }
    if (first_time || durability_service.history_kind != qos.m_durabilityService.history_kind ||
            durability_service.history_depth != qos.m_durabilityService.history_depth ||
            durability_service.max_instances != qos.m_durabilityService.max_instances ||
            durability_service.max_samples != qos.m_durabilityService.max_samples ||
            durability_service.max_samples_per_instance != qos.m_durabilityService.max_samples_per_instance ||
            durability_service.service_cleanup_delay != qos.m_durabilityService.service_cleanup_delay
            )
    {
        durability_service = qos.m_durabilityService;
        durability_service.hasChanged = true;
    }
    if (lifespan.duration != qos.m_lifespan.duration)
    {
        lifespan = qos.m_lifespan;
        lifespan.hasChanged = true;
    }
    if (qos.m_ownershipStrength.value != ownership_strength.value)
    {
        ownership_strength = qos.m_ownershipStrength;
        ownership_strength.hasChanged = true;
    }
    if (first_time)
    {
        disable_positive_acks = qos.m_disablePositiveACKs;
        disable_positive_acks.hasChanged = true;
    }
    // Writers only manages the first element in the list of data representations.
    if (qos.representation.m_value.size() != representation.m_value.size() ||
            (qos.representation.m_value.size() > 0 && representation.m_value.size() > 0 &&
            *qos.representation.m_value.begin() != *representation.m_value.begin()))
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }
    if (first_time && !(data_sharing == qos.data_sharing))
    {
        data_sharing = qos.data_sharing;
        data_sharing.hasChanged = true;
    }
    if (first_time)
    {
        publish_mode = qos.m_publishMode;
        publish_mode.hasChanged = true;
    }
}

bool PublicationBuiltinTopicData::can_qos_be_updated(
        const PublicationBuiltinTopicData& qos) const
{
    bool updatable = true;
    if ( durability.kind != qos.durability.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Durability kind cannot be changed after the creation of a publisher.");
    }

    if (liveliness.kind !=  qos.liveliness.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a publisher.");
    }

    if (liveliness.lease_duration != qos.liveliness.lease_duration)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Liveliness lease duration cannot be changed after the creation of a publisher.");
    }

    if (liveliness.announcement_period != qos.liveliness.announcement_period)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Liveliness announcement cannot be changed after the creation of a publisher.");
    }

    if (reliability.kind != qos.reliability.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a publisher.");
    }
    if (ownership.kind != qos.ownership.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a publisher.");
    }
    if (destination_order.kind != qos.destination_order.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Destination order Kind cannot be changed after the creation of a publisher.");
    }
    if (data_sharing.kind() != qos.data_sharing.kind() ||
            data_sharing.domain_ids() != qos.data_sharing.domain_ids())
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Data sharing configuration cannot be changed after the creation of a publisher.");
    }
    return updatable;
}

void PublicationBuiltinTopicData::clear()
{
    key = BuiltinTopicKey_t{{0, 0, 0}};
    participant_key = BuiltinTopicKey_t{{0, 0, 0}};
    type_name = "";
    topic_name = "";
    topic_kind = NO_KEY;
    persistence_guid = c_Guid_Unknown;
    guid = c_Guid_Unknown;
    remote_locators.unicast.clear();
    remote_locators.multicast.clear();
    loopback_transformation = NetworkConfigSet_t();
    max_serialized_size = 0;

    durability.clear();
    deadline.clear();
    latency_budget.clear();
    liveliness.clear();
    reliability.clear();
    ownership.clear();
    destination_order.clear();
    user_data.clear();
    presentation.clear();
    partition.clear();
    topic_data.clear();
    group_data.clear();
    durability_service.clear();
    lifespan.clear();
    disable_positive_acks.clear();
    ownership_strength.clear();
    representation.clear();
    data_sharing.clear();
    publish_mode.clear();

    reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
}

}   // namespace rtps
}   // namespace fastdds
}   // namespace eprosima
