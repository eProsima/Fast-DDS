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
 * @file SubscriptionBuiltinTopicData.cpp
 */

#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

void SubscriptionBuiltinTopicData::set_qos(
        const SubscriptionBuiltinTopicData& qos,
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
    if (time_based_filter.minimum_separation != qos.time_based_filter.minimum_separation )
    {
        time_based_filter = qos.time_based_filter;
        time_based_filter.hasChanged = true;
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
    if (lifespan.duration != qos.lifespan.duration )
    {
        lifespan = qos.lifespan;
        lifespan.hasChanged = true;
    }
    if (first_time)
    {
        disable_positive_acks = qos.disable_positive_acks;
        disable_positive_acks.hasChanged = true;
    }

    if (representation.m_value != qos.representation.m_value)
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }

    if (first_time ||
            type_consistency.m_kind != qos.type_consistency.m_kind ||
            type_consistency.m_ignore_member_names != qos.type_consistency.m_ignore_member_names ||
            type_consistency.m_ignore_string_bounds != qos.type_consistency.m_ignore_string_bounds ||
            type_consistency.m_ignore_sequence_bounds != qos.type_consistency.m_ignore_sequence_bounds ||
            type_consistency.m_force_type_validation != qos.type_consistency.m_force_type_validation ||
            type_consistency.m_prevent_type_widening != qos.type_consistency.m_prevent_type_widening)
    {
        type_consistency = qos.type_consistency;
        type_consistency.hasChanged = true;
    }

    if (!(data_sharing == qos.data_sharing))
    {
        data_sharing = qos.data_sharing;
        data_sharing.hasChanged = true;
    }
}

void SubscriptionBuiltinTopicData::set_qos(
        const dds::ReaderQos& qos,
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
    if (time_based_filter.minimum_separation != qos.m_timeBasedFilter.minimum_separation )
    {
        time_based_filter = qos.m_timeBasedFilter;
        time_based_filter.hasChanged = true;
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
    if (lifespan.duration != qos.m_lifespan.duration )
    {
        lifespan = qos.m_lifespan;
        lifespan.hasChanged = true;
    }
    if (first_time)
    {
        disable_positive_acks = qos.m_disablePositiveACKs;
        disable_positive_acks.hasChanged = true;
    }

    if (representation.m_value != qos.representation.m_value)
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }

    if (first_time ||
            type_consistency.m_kind != qos.type_consistency.m_kind ||
            type_consistency.m_ignore_member_names != qos.type_consistency.m_ignore_member_names ||
            type_consistency.m_ignore_string_bounds != qos.type_consistency.m_ignore_string_bounds ||
            type_consistency.m_ignore_sequence_bounds != qos.type_consistency.m_ignore_sequence_bounds ||
            type_consistency.m_force_type_validation != qos.type_consistency.m_force_type_validation ||
            type_consistency.m_prevent_type_widening != qos.type_consistency.m_prevent_type_widening)
    {
        type_consistency = qos.type_consistency;
        type_consistency.hasChanged = true;
    }

    if (!(data_sharing == qos.data_sharing))
    {
        data_sharing = qos.data_sharing;
        data_sharing.hasChanged = true;
    }
}

bool SubscriptionBuiltinTopicData::can_qos_be_updated(
        const SubscriptionBuiltinTopicData& qos) const
{
    bool updatable = true;
    if ( durability.kind != qos.durability.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Durability kind cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.kind != qos.liveliness.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.lease_duration != qos.liveliness.lease_duration)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Liveliness lease duration cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.announcement_period != qos.liveliness.announcement_period)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Liveliness announcement cannot be changed after the creation of a subscriber.");
    }

    if (reliability.kind != qos.reliability.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a subscriber.");
    }
    if (ownership.kind != qos.ownership.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a subscriber.");
    }
    if (destination_order.kind != qos.destination_order.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Destination order Kind cannot be changed after the creation of a subscriber.");
    }
    if (data_sharing.kind() != qos.data_sharing.kind() ||
            data_sharing.domain_ids() != qos.data_sharing.domain_ids())
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Data sharing configuration cannot be changed after the creation of a subscriber.");
    }
    return updatable;
}

void SubscriptionBuiltinTopicData::clear()
{
    key = BuiltinTopicKey_t{{0, 0, 0}};
    participant_key = BuiltinTopicKey_t{{0, 0, 0}};
    type_name = "";
    topic_name = "";
    topic_kind = NO_KEY;
    content_filter.filter_class_name = "";
    content_filter.content_filtered_topic_name = "";
    content_filter.related_topic_name = "";
    content_filter.filter_expression = "";
    content_filter.expression_parameters.clear();
    guid = c_Guid_Unknown;
    participant_guid = c_Guid_Unknown;
    remote_locators.unicast.clear();
    remote_locators.multicast.clear();
    loopback_transformation = NetworkConfigSet_t();
    expects_inline_qos = false;

    durability.clear();
    deadline.clear();
    latency_budget.clear();
    liveliness.clear();
    reliability.clear();
    ownership.clear();
    destination_order.clear();
    user_data.clear();
    time_based_filter.clear();
    presentation.clear();
    partition.clear();
    topic_data.clear();
    group_data.clear();
    lifespan.clear();
    disable_positive_acks.clear();
    representation.clear();
    type_consistency.clear();
    data_sharing.clear();
}

}   // namespace rtps
}   // namespace fastdds
}   // namespace eprosima
